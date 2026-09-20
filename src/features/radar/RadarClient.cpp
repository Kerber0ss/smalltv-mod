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
    case RADAR_NO_LOCATION: return "choose oblast and district";
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

static bool parseSituation(const Settings& s, Stream& stream) {
  // The service returns area and selected-district status separately. Keep
  // only the selected district fields needed by the display on the ESP8266.
  JsonDocument filter;
  JsonObject districtFilter = filter["district"].to<JsonObject>();
  districtFilter["name"] = true;
  districtFilter["level"] = true;
  districtFilter["counts"]["uav"] = true;
  districtFilter["counts"]["missile"] = true;
  districtFilter["counts"]["ballistic"] = true;

  JsonDocument doc;
  if (deserializeJson(doc, stream, DeserializationOption::Filter(filter))) {
    g_stage = RADAR_PARSE_FAIL;
    return false;
  }
  JsonObjectConst district = doc["district"].as<JsonObjectConst>();
  if (district.isNull()) { g_stage = RADAR_PARSE_FAIL; return false; }

  RadarSituation next{};
  strlcpy(next.placeName, district["name"] | s.radar.district, sizeof(next.placeName));
  const char* apiLevel = district["level"] | "";
  if (!strcmp(apiLevel, "red")) next.level = 2;
  else if (!strcmp(apiLevel, "yellow")) next.level = 1;
  else if (!strcmp(apiLevel, "green")) next.level = 0;
  else { g_stage = RADAR_PARSE_FAIL; return false; }
  next.drones = district["counts"]["uav"] | 0;
  next.missiles = (uint16_t)((district["counts"]["missile"] | 0) +
                             (district["counts"]["ballistic"] | 0));
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
  String url = String(F(RADAR_API_URL));
  url.reserve(url.length() + strlen(s.radar.region) + strlen(s.radar.district) * 3 + 12);
  const char hex[] = "0123456789ABCDEF";
  auto addQueryValue = [&](const char* value) {
    for (const unsigned char* p = (const unsigned char*)value; *p; ++p) {
      const unsigned char c = *p;
      if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
          (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
        url += (char)c;
      } else {
        url += '%';
        url += hex[c >> 4];
        url += hex[c & 0x0F];
      }
    }
  };
  addQueryValue(s.radar.region);
  url += F("&district=");
  addQueryValue(s.radar.district);
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
  if (!s.radar.region[0] || !s.radar.district[0]) { g_stage = RADAR_NO_LOCATION; return; }
  if ((int32_t)(millis() - g_nextPollMs) < 0) return;
  g_nextPollMs = millis() + (uint32_t)s.radar.pollSec * 1000UL;
  g_lastTryMs = millis();
  if (!fetch(s)) g_error = true;  // preserve the last complete snapshot on a transient error
}
