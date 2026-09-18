#include "RadarClient.h"
#include "Platform.h"
#include <ArduinoJson.h>

static RadarSituation g_data;
static uint32_t g_lastOkMs = 0, g_lastTryMs = 0, g_nextPollMs = 0;
static bool g_error = false;
static RadarStage g_stage = RADAR_IDLE;
static int g_lastHttp = 0;
static TlsSession g_tlsSession;

const RadarSituation& radarSituation() { return g_data; }
uint32_t radarLastOkMs() { return g_lastOkMs; }
bool radarError() { return g_error; }
RadarStage radarStage() { return g_stage; }
int radarLastHttp() { return g_lastHttp; }
uint32_t radarLastTryMs() { return g_lastTryMs; }

const char* radarStageName() {
  switch (g_stage) {
    case RADAR_NO_REGION: return "choose a region";
    case RADAR_LOW_HEAP: return "skipped, low heap";
    case RADAR_CONNECT_FAIL: return "connect failed";
    case RADAR_HTTP_ERROR: return "http error";
    case RADAR_PARSE_FAIL: return "parse failed";
    case RADAR_OK: return "ok";
    default: return "idle";
  }
}

void radarInit(const Settings& s) {
  (void)s;
  memset(&g_data, 0, sizeof(g_data));
  g_lastOkMs = g_lastTryMs = 0;
  g_nextPollMs = millis();
  g_error = false;
  g_stage = RADAR_IDLE;
  g_lastHttp = 0;
}

void radarForceRefresh() { g_nextPollMs = millis(); }

static bool samePlace(const char* wanted, const char* actual) {
  return wanted[0] && actual && !strcmp(wanted, actual);
}

static uint16_t addCount(uint16_t total, JsonObjectConst t) {
  int n = t["count"] | 1;
  if (n < 1) n = 1;
  uint32_t sum = (uint32_t)total + (uint32_t)n;
  return sum > 65535U ? 65535U : (uint16_t)sum;
}

static bool parseSituation(const Settings& s, Stream& stream) {
  // Filter every field not used by the 240px display. Keeping only target type,
  // place and group size bounds the temporary JSON tree on the ESP8266.
  JsonDocument filter;
  JsonObject r = filter["region"].to<JsonObject>();
  r["name"] = true;
  r["level"] = true;
  r["counts"]["uav"] = true;
  r["counts"]["missile"] = true;
  r["counts"]["ballistic"] = true;
  JsonObject raion = r["raions_under_alert"][0].to<JsonObject>();
  raion["name"] = true;
  JsonObject threat = r["threats"][0].to<JsonObject>();
  threat["type"] = true;
  threat["district"] = true;
  threat["locality"] = true;
  threat["count"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, stream, DeserializationOption::Filter(filter))) {
    g_stage = RADAR_PARSE_FAIL;
    return false;
  }
  JsonObjectConst region = doc["region"].as<JsonObjectConst>();
  if (region.isNull()) { g_stage = RADAR_PARSE_FAIL; return false; }

  RadarSituation next{};
  strlcpy(next.regionName, region["name"] | s.radar.region, sizeof(next.regionName));
  const bool local = s.radar.localScope && (s.radar.locality[0] || s.radar.district[0]);
  const char* wanted = s.radar.locality[0] ? s.radar.locality : s.radar.district;
  strlcpy(next.placeName, local ? wanted : next.regionName, sizeof(next.placeName));

  JsonArrayConst raions = region["raions_under_alert"].as<JsonArrayConst>();
  const char* apiLevel = region["level"] | "green";
  bool regionalAlert = !strcmp(apiLevel, "red") && raions.size() == 0;
  bool localAlert = regionalAlert;
  if (local && !regionalAlert && s.radar.district[0]) {
    for (JsonObjectConst raion : raions)
      if (samePlace(s.radar.district, raion["name"] | "")) { localAlert = true; break; }
  }

  uint16_t localThreats = 0;
  for (JsonObjectConst t : region["threats"].as<JsonArrayConst>()) {
    if (local && !samePlace(wanted, s.radar.locality[0] ? (t["locality"] | "")
                                                      : (t["district"] | ""))) continue;
    const char* type = t["type"] | "";
    if (!strcmp(type, "uav")) next.drones = addCount(next.drones, t);
    if (!strcmp(type, "missile") || !strcmp(type, "ballistic")) next.missiles = addCount(next.missiles, t);
    localThreats = addCount(localThreats, t);
  }

  if (!local) {
    next.drones = region["counts"]["uav"] | 0;
    next.missiles = (uint16_t)((region["counts"]["missile"] | 0) +
                               (region["counts"]["ballistic"] | 0));
    next.level = !strcmp(apiLevel, "red") ? 2 : !strcmp(apiLevel, "yellow") ? 1 : 0;
  } else {
    next.level = localAlert ? 2 : localThreats ? 1 : 0;
  }
  next.valid = true;
  g_data = next;
  g_lastOkMs = millis();
  g_error = false;
  g_stage = RADAR_OK;
  return true;
}

static bool fetch(const Settings& s) {
  if (platformMaxFreeBlock() < RADAR_TLS_MIN_BLOCK) {
    g_stage = RADAR_LOW_HEAP;
    return false;
  }
  std::unique_ptr<NetClient> client(platformMakeSecureClient(RADAR_TLS_RXBUF, &g_tlsSession));
  HTTPClient http;
  http.useHTTP10(true);                 // stream raw JSON, never chunk framing
  http.setTimeout(s.httpTimeout);
  http.setReuse(false);
  String url = String(F(RADAR_API_URL)) + s.radar.region;
  if (!http.begin(*client, url)) { g_stage = RADAR_CONNECT_FAIL; return false; }
  http.addHeader("Accept", "application/json");
  http.setUserAgent(F("SmallTV radar"));
  int code = http.GET();
  g_lastHttp = code;
  if (code != HTTP_CODE_OK) {
    http.end();
    g_stage = code < 0 ? RADAR_CONNECT_FAIL : RADAR_HTTP_ERROR;
    return false;
  }
  bool ok = parseSituation(s, http.getStream());
  http.end();
  return ok;
}

void radarService(const Settings& s) {
  if (!s.radar.region[0]) { g_stage = RADAR_NO_REGION; return; }
  if ((int32_t)(millis() - g_nextPollMs) < 0) return;
  g_nextPollMs = millis() + (uint32_t)s.radar.pollSec * 1000UL;
  g_lastTryMs = millis();
  if (!fetch(s)) g_error = true;  // preserve the last complete snapshot on a transient error
}
