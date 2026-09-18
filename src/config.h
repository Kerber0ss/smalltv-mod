// config.h — compile-time constants for smalltv-mod
//
// Hardware: GeekMagic SmallTV, ESP-12F (ESP8266), 1.54" 240x240 ST7789 IPS.
#pragma once

// ---------------------------------------------------------------------------
// Firmware identity
// ---------------------------------------------------------------------------
#define FW_NAME     "smalltv-mod"
#define FW_VERSION  "2.18.0"

// Project / update references (shown in the web UI; used by the GitHub self-update)
#define REPO_URL      "https://github.com/Kerber0ss/smalltv-mod"
#define REPO_OWNER    "Kerber0ss"
#define REPO_NAME     "smalltv-mod"
// The optional lean ESP8266 image keeps its own update stream.
#if defined(SMALLTV_LEAN)
  #define UPDATE_ASSET "smalltv-mod-firmware-lean.bin"
  #define FW_VARIANT   "esp8266-lean"
#else
  #define UPDATE_ASSET "smalltv-mod-firmware.bin"
  #define FW_VARIANT   "esp8266"
#endif
#define GH_API_HOST   "api.github.com"
#define DAEMON_URL    "https://github.com/giovi321/clawdmeter-daemon"

// ---------------------------------------------------------------------------
// Display wiring + panel quirks.
// Provides TFT_SCLK/MOSI/DC/RST/CS/BL, TFT_BGR, TFT_BL_DEFAULT_INVERTED,
// HAS_LDR/LDR_PIN/ADC_MAX. Both panels are 1.54" 240x240 ST7789 IPS.
// ---------------------------------------------------------------------------
#include "board_esp8266.h"

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// Panel RAM offsets per rotation pair (Arduino_GFX: offset1 -> rotation 0/1,
// offset2 -> rotation 2/3). The ST7789(V) is a 240x320 controller but the
// 1.54" glass only wires RAM rows 0-239, leaving an 80-row dead band. With
// the MADCTL MY bit set (rotation 2/3) the scan direction reverses into that
// dead band, so the row offset must jump to 80 or the image slides 80 px off
// the glass (content pushed to the top at 180°). #ifndef so a board header
// can override if a variant turns up with a 240x280 panel (would need 40).
#ifndef TFT_COL_OFFSET1
#define TFT_COL_OFFSET1 0
#endif
#ifndef TFT_ROW_OFFSET1
#define TFT_ROW_OFFSET1 0
#endif
#ifndef TFT_COL_OFFSET2
#define TFT_COL_OFFSET2 0
#endif
#ifndef TFT_ROW_OFFSET2
#define TFT_ROW_OFFSET2 80
#endif

// ---------------------------------------------------------------------------
// Limits (bound RAM usage on the ESP8266)
// ---------------------------------------------------------------------------
#define MAX_WIFI_NETS     4    // saved WiFi networks; strongest visible wins at boot

// ---------------------------------------------------------------------------
// Web UI password (off by default). Digest auth, so the password itself is
// never sent over the wire even though the UI is plain HTTP.
// ---------------------------------------------------------------------------
#define MAX_AUTH_USER_LEN 32
#define MAX_AUTH_PASS_LEN 64
#define DEFAULT_AUTH_USER "admin"
#define AUTH_REALM        "SmallTV"

// ---------------------------------------------------------------------------
// Display mode — what the device shows
//   0 = Claude usage meter (mascot + 5h/7d usage bars, fed by the daemon/)
//   1 = air-alert radar
//   2 = carousel: rotate through the selected features on a timer
// ---------------------------------------------------------------------------
#define MODE_USAGE     0
#define MODE_RADAR     1
#define MODE_CAROUSEL  2
#define MODE_NOTIFY    3             // transient overlay: armed over HTTP, never persisted
#define MODE_HA        4             // Home Assistant screens pushed over MQTT
#define DEFAULT_MODE MODE_USAGE
#define DEFAULT_CAROUSEL_SEC 30      // per-mode dwell in carousel

// Full-screen attention overlay (POST /api/notify), in seconds.
#define NOTIFY_TTL_DEFAULT_SEC  20
#define NOTIFY_TTL_MIN_SEC       2
#define NOTIFY_TTL_MAX_SEC     120

// Pending overlays wait in a fixed-size queue rather than overwriting each other,
// so a burst shows every event in turn instead of only the last one. Depth and
// label length are the whole RAM cost of the feature, so keep the small pair.
#define NOTIFY_QUEUE_DEPTH       2
#define NOTIFY_LABEL_MAX        48
#define NOTIFY_TITLE_MAX        20   // the word above the label, one panel line

// Priority decides queue order and pre-emption: a strictly higher one jumps the
// queue and takes the panel from whatever is on it. Values are what the API's
// "priority" field accepts, so they are part of the contract.
#define NOTIFY_PRIO_LOW          0
#define NOTIFY_PRIO_MED          1
#define NOTIFY_PRIO_HIGH         2
#define NOTIFY_PRIO_MAX          3

// Marquee for labels wider than the panel: pixels per step and step interval.
// A character is GFX_FONT_W * NOTIFY_LABEL_SIZE = 12 px wide, so 4 px every
// 40 ms is 100 px/s, or 120 ms per character. At that speed a 90-character
// label (1108 px including the gap) takes about 11 s for one pass, so the
// default 20 s hold shows it just under twice. Half this speed did not finish a
// single pass within the default hold. The step is what changed, not the
// interval: the band is redrawn at the same 25 Hz as before.
#define NOTIFY_SCROLL_STEP_PX    4
#define NOTIFY_SCROLL_MS        40
#define NOTIFY_SCROLL_GAP_PX    28   // blank run between the end and the repeat
#define NOTIFY_SCROLL_PAUSE_MS 900   // hold at the start before scrolling begins

// ---------------------------------------------------------------------------
// Home Assistant screens (MODE_HA, features/ha): full screens pushed over MQTT
// as retained JSON draw lists, one per slot, on smalltv/<hostname>/screen/<slot>.
// Heap is the binding constraint. MQTT_MAX_PACKET_SIZE reaches PubSubClient
// through platformio.ini build_flags.
// ---------------------------------------------------------------------------
#define HA_MAX_SCREENS   4
#define HA_MAX_PRIMS     24
#define HA_TEXT_POOL     512    // the 768 B MQTT payload bounds text anyway
#define HA_BITMAP_POOL   512
#define HA_BITMAP_MAX_DIM 32    // one bitmap is <= 128 B (256 hex chars)
#define HA_MAX_TEXT        65     // one text value: 64 chars + NUL
#define HA_SLOT_LEN        25     // slot (topic suffix) cap: 24 chars + NUL
#define HA_TTL_MAX_SEC     604800UL  // ttl clamp: 7 days (keeps millis() math sane)
#define HA_PERSIST_DEBOUNCE_MS 2000UL  // /ha_screens.json write delay after a change

// Broker settings slice (Settings.h): field caps + defaults.
#define MAX_HA_HOST_LEN        64
#define MAX_HA_USER_LEN        32
#define MAX_HA_PASS_LEN        32
#define DEFAULT_HA_BROKER_PORT 1883
#define DEFAULT_HA_DWELL_SEC   15
#define HA_DWELL_MIN_SEC        3
#define HA_DWELL_MAX_SEC      300

// ---------------------------------------------------------------------------
// Compile-time feature toggles. All shipping features are on by default; a lean
// build drops one by setting e.g. -D WITH_RADAR=0 in a PlatformIO env, which
// omits that feature's module from the registry and its web UI section.
// ---------------------------------------------------------------------------
#ifndef WITH_USAGE
#define WITH_USAGE 1
#endif
#ifndef WITH_RADAR
#define WITH_RADAR 1
#endif
#ifndef WITH_NOTIFY
#define WITH_NOTIFY 1
#endif
#ifndef WITH_HA
#define WITH_HA 1
#endif

// Claude usage mode: once data stops arriving for this long (PC asleep, daemon
// stopped, network down) the screen switches from the stats to the idle mascot
// animation. Effective timeout also scales with the poll period (see main.cpp).
#define USAGE_STALE_GRACE_MS  20000UL

// ---------------------------------------------------------------------------
// Air-alert radar (MODE_RADAR). The API is region-filtered before it reaches
// the ESP8266; only the selected region's situation is parsed from its stream.
// ---------------------------------------------------------------------------
#define RADAR_API_URL          "https://radar.syslog.pp.ua/v1/situation?region="
#define RADAR_TLS_RXBUF        4096
#define RADAR_TLS_MIN_BLOCK    16000
#define MAX_RADAR_REGION_LEN   24
#define MAX_RADAR_LOCATION_LEN 48
#define DEFAULT_RADAR_REGION   ""
#define DEFAULT_RADAR_POLL_SEC 30

// ---------------------------------------------------------------------------
// Defaults (used on first boot / factory reset)
// ---------------------------------------------------------------------------
#define DEFAULT_AP_SSID      "SmallTV-Setup"
#define DEFAULT_AP_PASS      ""              // empty => open AP
#define DEFAULT_HOSTNAME     "smalltv"
#define DEFAULT_POLL_SEC      120            // how often to refresh data
#define DEFAULT_BRIGHTNESS    90             // 0..100 %
#define DEFAULT_HTTP_TIMEOUT  8000           // ms per request

// --- Panel colour correction (device-wide) ---
// Panels differ between (and within) the SmallTV variants: white balance drifts
// and some controllers have red and blue swapped. AUTO keeps the board header's
// TFT_BGR default; RGB/BGR force the MADCTL colour-order bit either way.
#define COLOR_ORDER_AUTO   0
#define COLOR_ORDER_RGB    1
#define COLOR_ORDER_BGR    2
#define DEFAULT_COLOR_ORDER  COLOR_ORDER_AUTO
#define DEFAULT_COLOR_INVERT false
#define DEFAULT_COLOR_GAIN   100     // percent per channel; 50..150 accepted
#define MIN_COLOR_GAIN        50
#define MAX_COLOR_GAIN       150

// --- Clock / night mode (device-wide) ---
#define NTP_SERVER1             "pool.ntp.org"
#define NTP_SERVER2             "time.nist.gov"
#define DEFAULT_TZ_NAME         ""        // IANA display name; empty = UTC
#define DEFAULT_TZ_POSIX        "UTC0"    // POSIX TZ rule the device feeds SNTP
#define DEFAULT_NIGHT_ENABLED   false
#define DEFAULT_NIGHT_START_MIN 1320      // 22:00
#define DEFAULT_NIGHT_END_MIN   420       // 07:00
#define DEFAULT_NIGHT_LEVEL     0         // 0..100, 0 = backlight fully off

// Night-mode NTP trust: only ENTER night mode when the clock was confirmed by a
// successful NTP sync within NIGHT_NTP_TRUST_MS (else we assume the clock may be
// wrong and keep the screen on). While inside the window but unconfirmed, re-arm
// SNTP every NIGHT_NTP_RESYNC_MS until a fresh sync lands or the window ends
// (morning). Once night mode has switched on, it stays on until the window ends.
#define NIGHT_NTP_TRUST_MS      300000UL  // 5 min: max age of the sync that unlocks night
#define NIGHT_NTP_RESYNC_MS      30000UL  // re-sync attempt cadence while held off
