// Settings.h — persisted configuration (LittleFS /config.json)
//
// Layout is segmented per feature: shared device/network fields live at the top
// level, and each feature owns a nested settings slice (usage / radar / HA).
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

// One saved WiFi station network. The device keeps up to MAX_WIFI_NETS and
// joins the strongest visible one at boot (hidden SSIDs are tried last).
struct WifiCred {
  String ssid;
  String pass;
};

// ---- Usage feature slice ---------------------------------------------------
struct UsageSettings {
  String   usageUrl;        // optional Claude daemon HTTP endpoint
  uint16_t pollSec;         // Claude pull refresh period
  uint16_t rotateSec;       // dwell when both screens are enabled
  bool     claudeEnabled;
  bool     codexEnabled;

  void setDefaults();
  void toJson(JsonObject o) const;
  void fromJson(JsonObjectConst o);
};

// ---- Clock / night mode slice (device-wide) --------------------------------
struct ClockSettings {
  String   tz;            // IANA display name, e.g. "Europe/Rome" (UI round-trip)
  String   tzPosix;       // POSIX TZ rule the device feeds SNTP
  bool     nightEnabled;  // dim/blank on a nightly schedule
  uint16_t nightStartMin; // minutes since local midnight (0..1439)
  uint16_t nightEndMin;
  uint8_t  nightLevel;    // 0..100, 0 = backlight off

  void setDefaults();
  void toJson(JsonObject o) const;
  void fromJson(JsonObjectConst o);
};

// ---- Web UI password slice (device-wide) -----------------------------------
// Off by default: the settings page is open on the LAN, as it always was.
// Turning it on puts every page and every API endpoint behind HTTP digest auth,
// with the exceptions listed in WebPortal.cpp. Like the other secrets here, the
// password reaches the config file and the settings export, never the web API.
struct AuthSettings {
  bool   enabled;
  String user;
  String pass;

  void setDefaults();
  void toJson(JsonObject o, bool includeSecrets) const;
  void fromJson(JsonObjectConst o);
};

// ---- Panel colour slice (device-wide) --------------------------------------
// Same firmware, different panels: the SmallTV variants and even units of one
// variant render the same RGB565 value differently, and a few have red and blue
// swapped in the controller. These settings correct that per device.
struct DisplaySettings {
  uint8_t colorOrder;   // COLOR_ORDER_AUTO / _RGB / _BGR (auto = board default)
  bool    invert;       // flip the panel's inversion bit (washed-out / negative panels)
  uint8_t rGain;        // per-channel gain in percent, 50..150, 100 = untouched
  uint8_t gGain;
  uint8_t bGain;

  void setDefaults();
  void toJson(JsonObject o) const;
  void fromJson(JsonObjectConst o);
};

// ---- Home Assistant / MQTT feature slice -----------------------------------
// The LAN broker the device connects to for HA screens (features/ha). Plain
// TCP, no TLS. brokerPass follows the same rule as the other secrets here: it
// reaches the config file and the settings export, never the web API.
struct HaSettings {
  String   brokerHost;    // MQTT broker hostname or IP; empty = feature off
  uint16_t brokerPort;
  String   brokerUser;    // optional
  String   brokerPass;    // optional secret
  uint16_t dwellSec;      // per-screen time in the HA rotation

  void setDefaults();
  void toJson(JsonObject o, bool includeSecrets) const;
  void fromJson(JsonObjectConst o);
};

// ---- Air-alert radar feature slice ----------------------------------------
struct RadarSettings {
  char     region[MAX_RADAR_REGION_LEN];      // API region key, empty = not configured
  char     district[MAX_RADAR_LOCATION_LEN];  // required API district key
  uint16_t pollSec;                           // refresh period

  void setDefaults();
  void toJson(JsonObject o) const;
  void fromJson(JsonObjectConst o);
};

// ---- Top-level settings ----------------------------------------------------
struct Settings {
  // --- WiFi station networks (the device joins one of these) ---
  WifiCred wifi[MAX_WIFI_NETS];
  uint8_t  wifiCount;

  // --- Access point (config / fallback hotspot) ---
  String apSsid;
  String apPass;        // empty => open network
  String hostname;      // mDNS name => http://<hostname>.local

  // --- Active feature ---
  uint8_t mode;         // MODE_USAGE / MODE_RADAR / MODE_CAROUSEL / MODE_HA

  // --- Carousel (mode == MODE_CAROUSEL): dwell + which features rotate ---
  uint16_t carouselSec;
  bool carouselUsage, carouselRadar, carouselHa;

  // --- Shared HTTP / display ---
  uint16_t httpTimeout; // ms
  uint8_t  brightness;        // 0..100 %
  bool     autoBrightness;    // use LDR on A0
  bool     backlightInverted; // active-low backlight
  uint8_t  rotation;          // 0..3 screen orientation

  // --- Feature slices ---
  UsageSettings   usage;
  RadarSettings   radar;
  HaSettings      ha;        // MQTT broker for HA screens
  ClockSettings   clock;
  DisplaySettings display;   // panel colour correction
  AuthSettings    auth;      // optional web UI password

  void setDefaults();
};

// Persistence
bool settingsBegin();                       // mount LittleFS
bool loadSettings(Settings& s);             // false => defaults applied
bool saveSettings(const Settings& s);
void factoryReset(Settings& s);             // wipe file + defaults

// JSON <-> struct. `includeSecrets=false` masks passwords for the web API.
void settingsToJson(const Settings& s, JsonObject root, bool includeSecrets);
void settingsApplyJson(Settings& s, JsonObjectConst root); // partial update allowed
