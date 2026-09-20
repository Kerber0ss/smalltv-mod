#include "Settings.h"
#include "Platform.h"   // platformChipId() for the unique default hostname
#include <LittleFS.h>

static const char* CONFIG_PATH = "/config.json";

// ===========================================================================
// Usage slice
// ===========================================================================
void UsageSettings::setDefaults() {
  usageUrl = "";
  pollSec = DEFAULT_POLL_SEC;
  rotateSec = 30;
  claudeEnabled = true;
  codexEnabled = false;  // preserves the old single-Claude-screen behaviour on upgrade
}

void UsageSettings::toJson(JsonObject o) const {
  o["usageUrl"]      = usageUrl;
  o["pollSec"]       = pollSec;
  o["rotateSec"]     = rotateSec;
  o["claudeEnabled"] = claudeEnabled;
  o["codexEnabled"]  = codexEnabled;
}

void UsageSettings::fromJson(JsonObjectConst o) {
  if (o["usageUrl"].is<const char*>()) usageUrl = o["usageUrl"].as<String>();
  if (o["pollSec"].is<int>())          pollSec = constrain((int)o["pollSec"], 10, 3600);
  if (o["rotateSec"].is<int>())        rotateSec = constrain((int)o["rotateSec"], 2, 3600);
  if (o["claudeEnabled"].is<bool>())   claudeEnabled = o["claudeEnabled"];
  if (o["codexEnabled"].is<bool>())    codexEnabled = o["codexEnabled"];
}

// ===========================================================================
// Clock / night mode slice
// ===========================================================================
static uint16_t hhmmToMin(const char* s, uint16_t fallback) {
  if (!s || !s[0]) return fallback;
  int h = 0, m = 0;
  if (sscanf(s, "%d:%d", &h, &m) != 2) return fallback;
  if (h < 0 || h > 23 || m < 0 || m > 59) return fallback;
  return (uint16_t)(h * 60 + m);
}
static String minToHhmm(uint16_t v) {
  if (v > 1439) v = 0;
  char b[6];
  snprintf(b, sizeof(b), "%02u:%02u", (unsigned)(v / 60), (unsigned)(v % 60));
  return String(b);
}

void ClockSettings::setDefaults() {
  tz            = DEFAULT_TZ_NAME;
  tzPosix       = DEFAULT_TZ_POSIX;
  nightEnabled  = DEFAULT_NIGHT_ENABLED;
  nightStartMin = DEFAULT_NIGHT_START_MIN;
  nightEndMin   = DEFAULT_NIGHT_END_MIN;
  nightLevel    = DEFAULT_NIGHT_LEVEL;
}

void ClockSettings::toJson(JsonObject o) const {
  o["tz"]           = tz;
  o["tzPosix"]      = tzPosix;
  o["nightEnabled"] = nightEnabled;
  o["nightStart"]   = minToHhmm(nightStartMin);
  o["nightEnd"]     = minToHhmm(nightEndMin);
  o["nightLevel"]   = nightLevel;
}

void ClockSettings::fromJson(JsonObjectConst o) {
  if (o["tz"].is<const char*>())          tz = o["tz"].as<String>();
  if (o["tzPosix"].is<const char*>())     tzPosix = o["tzPosix"].as<String>();
  if (o["nightEnabled"].is<bool>())       nightEnabled = o["nightEnabled"];
  if (o["nightStart"].is<const char*>())  nightStartMin = hhmmToMin(o["nightStart"], nightStartMin);
  if (o["nightEnd"].is<const char*>())    nightEndMin   = hhmmToMin(o["nightEnd"], nightEndMin);
  if (o["nightLevel"].is<int>())          nightLevel = constrain((int)o["nightLevel"], 0, 100);
}

// ===========================================================================
// Web UI password slice
// ===========================================================================
void AuthSettings::setDefaults() {
  enabled = false;
  user = DEFAULT_AUTH_USER;
  pass = "";
}

void AuthSettings::toJson(JsonObject o, bool includeSecrets) const {
  // Never enable in the saved config without a password to check against; a
  // blank one would lock the page with a credential nobody can supply.
  o["enabled"] = enabled && pass.length() > 0;
  o["user"]    = user;
  o["passSet"] = pass.length() > 0;
  if (includeSecrets) o["pass"] = pass;
}

void AuthSettings::fromJson(JsonObjectConst o) {
  if (o["user"].is<const char*>()) user = o["user"].as<String>();
  // Blank keeps the stored password, as everywhere else in this file.
  if (o["pass"].is<const char*>()) {
    String p = o["pass"].as<String>();
    if (p.length()) pass = p;
  }
  if (o["enabled"].is<bool>()) enabled = o["enabled"];
  if (user.length() >= MAX_AUTH_USER_LEN) user.remove(MAX_AUTH_USER_LEN - 1);
  if (pass.length() >= MAX_AUTH_PASS_LEN) pass.remove(MAX_AUTH_PASS_LEN - 1);
  if (!user.length()) user = DEFAULT_AUTH_USER;
  // Same guard as toJson, for a config imported by hand.
  if (!pass.length()) enabled = false;
}

// ===========================================================================
// Panel colour slice
// ===========================================================================
void DisplaySettings::setDefaults() {
  colorOrder = DEFAULT_COLOR_ORDER;
  invert     = DEFAULT_COLOR_INVERT;
  rGain = gGain = bGain = DEFAULT_COLOR_GAIN;
}

void DisplaySettings::toJson(JsonObject o) const {
  o["colorOrder"] = (colorOrder == COLOR_ORDER_RGB) ? "rgb"
                  : (colorOrder == COLOR_ORDER_BGR) ? "bgr" : "auto";
  o["invert"] = invert;
  o["rGain"]  = rGain;
  o["gGain"]  = gGain;
  o["bGain"]  = bGain;
}

void DisplaySettings::fromJson(JsonObjectConst o) {
  if (o["colorOrder"].is<const char*>()) {
    String c = o["colorOrder"].as<String>();
    colorOrder = c.equalsIgnoreCase("rgb") ? COLOR_ORDER_RGB
               : c.equalsIgnoreCase("bgr") ? COLOR_ORDER_BGR : COLOR_ORDER_AUTO;
  }
  if (o["invert"].is<bool>()) invert = o["invert"];
  if (o["rGain"].is<int>()) rGain = constrain((int)o["rGain"], MIN_COLOR_GAIN, MAX_COLOR_GAIN);
  if (o["gGain"].is<int>()) gGain = constrain((int)o["gGain"], MIN_COLOR_GAIN, MAX_COLOR_GAIN);
  if (o["bGain"].is<int>()) bGain = constrain((int)o["bGain"], MIN_COLOR_GAIN, MAX_COLOR_GAIN);
}

// ===========================================================================
// Home Assistant / MQTT slice
// ===========================================================================
void HaSettings::setDefaults() {
  brokerHost = "";
  brokerPort = DEFAULT_HA_BROKER_PORT;
  brokerUser = "";
  brokerPass = "";
  dwellSec   = DEFAULT_HA_DWELL_SEC;
}

void HaSettings::toJson(JsonObject o, bool includeSecrets) const {
  o["brokerHost"] = brokerHost;
  o["brokerPort"] = brokerPort;
  o["brokerUser"] = brokerUser;
  o["passSet"]    = brokerPass.length() > 0;
  if (includeSecrets) o["brokerPass"] = brokerPass;
  o["dwellSec"]   = dwellSec;
}

void HaSettings::fromJson(JsonObjectConst o) {
  if (o["brokerHost"].is<const char*>()) brokerHost = o["brokerHost"].as<String>();
  if (o["brokerPort"].is<int>())         brokerPort = constrain((int)o["brokerPort"], 1, 65535);
  if (o["brokerUser"].is<const char*>()) brokerUser = o["brokerUser"].as<String>();
  // Blank keeps the stored password, as everywhere else in this file.
  if (o["brokerPass"].is<const char*>()) {
    String p = o["brokerPass"].as<String>();
    if (p.length()) brokerPass = p;
  }
  if (o["dwellSec"].is<int>()) dwellSec = constrain((int)o["dwellSec"], HA_DWELL_MIN_SEC, HA_DWELL_MAX_SEC);
  if (brokerHost.length() >= MAX_HA_HOST_LEN) brokerHost.remove(MAX_HA_HOST_LEN - 1);
  if (brokerUser.length() >= MAX_HA_USER_LEN) brokerUser.remove(MAX_HA_USER_LEN - 1);
  if (brokerPass.length() >= MAX_HA_PASS_LEN) brokerPass.remove(MAX_HA_PASS_LEN - 1);
}

// ===========================================================================
// Radar slice
// ===========================================================================
void RadarSettings::setDefaults() {
  strlcpy(region, DEFAULT_RADAR_REGION, sizeof(region));
  district[0] = 0;
  pollSec = DEFAULT_RADAR_POLL_SEC;
}

void RadarSettings::toJson(JsonObject o) const {
  o["region"]      = region;
  o["district"]    = district;
  o["pollSec"]     = pollSec;
}

void RadarSettings::fromJson(JsonObjectConst o) {
  if (o["region"].is<const char*>()) strlcpy(region, o["region"], sizeof(region));
  if (o["district"].is<const char*>()) strlcpy(district, o["district"], sizeof(district));
  if (o["pollSec"].is<int>()) pollSec = constrain((int)o["pollSec"], 10, 3600);
}

// ===========================================================================
// Top-level settings
// ===========================================================================
void Settings::setDefaults() {
  wifiCount = 0;
  for (uint8_t i = 0; i < MAX_WIFI_NETS; i++) {
    wifi[i].ssid = "";
    wifi[i].pass = "";
  }
  apSsid  = DEFAULT_AP_SSID;
  apPass  = DEFAULT_AP_PASS;
  // Unique per device so several SmallTVs on one network don't collide on
  // mDNS out of the box. A hostname saved in config.json overrides this.
  hostname = String(DEFAULT_HOSTNAME) + "-" + String(platformChipId() & 0xFFFF, HEX);

  mode = DEFAULT_MODE;
  carouselSec = DEFAULT_CAROUSEL_SEC;
  carouselUsage = carouselRadar = carouselHa = true;
  httpTimeout = DEFAULT_HTTP_TIMEOUT;

  brightness = DEFAULT_BRIGHTNESS;
  autoBrightness = false;
  backlightInverted = TFT_BL_DEFAULT_INVERTED;
  rotation = 0;

  usage.setDefaults();
  radar.setDefaults();
  ha.setDefaults();
  clock.setDefaults();
  display.setDefaults();
  auth.setDefaults();
}

// ---------------------------------------------------------------------------
bool settingsBegin() {
  if (LittleFS.begin()) return true;
  // First boot on a fresh chip: format then mount.
  if (LittleFS.format() && LittleFS.begin()) return true;
  return false;
}

bool loadSettings(Settings& s) {
  s.setDefaults();
  File f = LittleFS.open(CONFIG_PATH, "r");
  if (!f) return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  settingsApplyJson(s, doc.as<JsonObjectConst>());
  return true;
}

bool saveSettings(const Settings& s) {
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  settingsToJson(s, root, /*includeSecrets=*/true);

  File f = LittleFS.open(CONFIG_PATH, "w");
  if (!f) return false;
  bool ok = serializeJson(doc, f) > 0;
  f.close();
  return ok;
}

void factoryReset(Settings& s) {
  LittleFS.remove(CONFIG_PATH);
  s.setDefaults();
}

// ---------------------------------------------------------------------------
void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets) {
  root["hostname"]   = s.hostname;

  // WiFi networks. Passwords only reach the config file, never the web API.
  JsonArray wf = root["wifi"].to<JsonArray>();
  for (uint8_t i = 0; i < s.wifiCount; i++) {
    JsonObject e = wf.add<JsonObject>();
    e["ssid"]    = s.wifi[i].ssid;
    e["passSet"] = s.wifi[i].pass.length() > 0;
    if (includeSecrets) e["pass"] = s.wifi[i].pass;
  }
  // Legacy mirror of the primary network, kept for one release so a firmware
  // downgrade still finds its WiFi in config.json.
  root["staSsid"]    = s.wifiCount ? s.wifi[0].ssid : "";
  root["staPassSet"] = s.wifiCount && s.wifi[0].pass.length() > 0;
  root["apSsid"]     = s.apSsid;
  root["apPassSet"]  = s.apPass.length() > 0;
  if (includeSecrets) {
    root["staPass"]  = s.wifiCount ? s.wifi[0].pass : "";
    root["apPass"]   = s.apPass;
  }

  // Mode + shared HTTP/display
  root["mode"]              = (s.mode == MODE_RADAR)    ? "radar"
                            : (s.mode == MODE_USAGE)    ? "usage"
                            : (s.mode == MODE_HA)       ? "ha"
                            : "carousel";
  root["carouselSec"]       = s.carouselSec;
  root["carouselUsage"]     = s.carouselUsage;
  root["carouselRadar"]     = s.carouselRadar;
  root["carouselHa"]        = s.carouselHa;
  root["httpTimeout"]       = s.httpTimeout;
  root["brightness"]        = s.brightness;
  root["autoBrightness"]    = s.autoBrightness;
  root["backlightInverted"] = s.backlightInverted;
  root["rotation"]          = s.rotation;

  // Feature slices
  s.usage.toJson(root["usage"].to<JsonObject>());
  s.radar.toJson(root["radar"].to<JsonObject>());
  s.ha.toJson(root["ha"].to<JsonObject>(), includeSecrets);
  s.clock.toJson(root["clock"].to<JsonObject>());
  s.display.toJson(root["display"].to<JsonObject>());
  s.auth.toJson(root["auth"].to<JsonObject>(), includeSecrets);
}

// Apply only the keys that are present (partial update friendly). Accepts both
// the nested layout and the legacy flat layout (feature keys at the top level).
void settingsApplyJson(Settings& s, JsonObjectConst root) {
  if (root["hostname"].is<const char*>()) s.hostname = root["hostname"].as<String>();

  if (root["wifi"].is<JsonArrayConst>()) {
    // The list is authoritative when present (order = try priority, missing
    // row = deletion). A blank password keeps the stored one, matched by SSID
    // so rows survive reordering.
    WifiCred old[MAX_WIFI_NETS];
    uint8_t oldCount = s.wifiCount;
    for (uint8_t i = 0; i < oldCount; i++) old[i] = s.wifi[i];

    s.wifiCount = 0;
    for (JsonObjectConst e : root["wifi"].as<JsonArrayConst>()) {
      if (s.wifiCount >= MAX_WIFI_NETS) break;
      const char* ssid = e["ssid"] | "";
      if (!ssid[0]) continue;                // skip blank rows
      WifiCred& dst = s.wifi[s.wifiCount];
      dst.ssid = ssid;
      const char* pass = e["pass"] | "";
      dst.pass = pass;
      if (!pass[0])
        for (uint8_t i = 0; i < oldCount; i++)
          if (old[i].ssid == dst.ssid) { dst.pass = old[i].pass; break; }
      s.wifiCount++;
    }
  } else if (root["staSsid"].is<const char*>()) {
    // Legacy single-network layout (pre-2.4 config.json or an old cached web
    // page): it becomes/updates the primary network, extras stay untouched.
    String ssid = root["staSsid"].as<String>();
    if (ssid.length()) {
      s.wifi[0].ssid = ssid;
      if (root["staPass"].is<const char*>()) {
        String p = root["staPass"].as<String>();
        if (p.length() > 0) s.wifi[0].pass = p;   // blank = keep
      }
      if (s.wifiCount < 1) s.wifiCount = 1;
    }
  }
  if (root["apSsid"].is<const char*>()) s.apSsid = root["apSsid"].as<String>();
  // AP password: apply as-is when present (empty allowed => open AP).
  if (root["apPass"].is<const char*>()) s.apPass = root["apPass"].as<String>();

  if (root["mode"].is<const char*>()) {
    String m = root["mode"].as<String>();
    s.mode = m.equalsIgnoreCase("radar")    ? MODE_RADAR
           : m.equalsIgnoreCase("usage")    ? MODE_USAGE
           : m.equalsIgnoreCase("ha")       ? MODE_HA
           : m.equalsIgnoreCase("carousel") ? MODE_CAROUSEL : MODE_USAGE;
  }
#if !WITH_RADAR
  if (s.mode == MODE_RADAR) s.mode = MODE_USAGE;
#endif
  if (root["carouselSec"].is<int>())      s.carouselSec = constrain((int)root["carouselSec"], 5, 3600);
  if (root["carouselUsage"].is<bool>())   s.carouselUsage = root["carouselUsage"];
  if (root["carouselRadar"].is<bool>())   s.carouselRadar = root["carouselRadar"];
  if (root["carouselHa"].is<bool>())      s.carouselHa = root["carouselHa"];

  if (root["httpTimeout"].is<int>())        s.httpTimeout = constrain((int)root["httpTimeout"], 1000, 20000);
  if (root["brightness"].is<int>())         s.brightness = constrain((int)root["brightness"], 0, 100);
  if (root["autoBrightness"].is<bool>())    s.autoBrightness = root["autoBrightness"];
  if (root["backlightInverted"].is<bool>()) s.backlightInverted = root["backlightInverted"];
  if (root["rotation"].is<int>())           s.rotation = (uint8_t)(((int)root["rotation"]) & 3);

  // Usage accepts the legacy flat layout; other features use nested objects.
  JsonObjectConst u = root["usage"].is<JsonObjectConst>() ? root["usage"].as<JsonObjectConst>() : root;
  s.usage.fromJson(u);
  // Radar has no legacy flat layout; only apply when its nested object is present.
  if (root["radar"].is<JsonObjectConst>()) s.radar.fromJson(root["radar"].as<JsonObjectConst>());
  // HA has no legacy flat layout either; only apply when its object is present.
  if (root["ha"].is<JsonObjectConst>()) s.ha.fromJson(root["ha"].as<JsonObjectConst>());
  if (root["clock"].is<JsonObjectConst>()) s.clock.fromJson(root["clock"].as<JsonObjectConst>());
  if (root["display"].is<JsonObjectConst>()) s.display.fromJson(root["display"].as<JsonObjectConst>());
  if (root["auth"].is<JsonObjectConst>()) s.auth.fromJson(root["auth"].as<JsonObjectConst>());
}
