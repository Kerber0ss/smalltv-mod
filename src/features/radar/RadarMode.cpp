#include "RadarMode.h"
#include <Arduino_GFX_Library.h>
#include "Gfx.h"
#include "RadarClient.h"

RadarMode g_radarMode;

// The built-in GFX font is ASCII-only. This tiny transliterator keeps selected
// Ukrainian/Russian places readable without carrying a multi-kilobyte font.
static void labelAscii(const char* in, char* out, size_t outSize) {
  static const char upper[] = "ABVGDEJZIYKLMNOPRSTUFHCSSYEEYUYA";
  static const char lower0[] = "abvgdejziyklmnop";
  static const char lower1[] = "rstufhcssyeeyuya";
  size_t n = 0;
  while (*in && n + 1 < outSize) {
    uint8_t c = (uint8_t)*in++;
    if (c < 0x80) { out[n++] = c; continue; }
    if (!*in) break;  // malformed/truncated UTF-8 must not read beyond the label
    uint8_t d = (uint8_t)*in++;
    char x = '?';
    if (c == 0xD0 && d >= 0x90 && d <= 0xAF) x = upper[d - 0x90];
    else if (c == 0xD0 && d >= 0xB0 && d <= 0xBF) x = lower0[d - 0xB0];
    else if (c == 0xD1 && d >= 0x80 && d <= 0x8F) x = lower1[d - 0x80];
    else if (c == 0xD0 && d == 0x81) x = 'E';
    else if (c == 0xD1 && d == 0x91) x = 'e';
    else if (c == 0xD0 && d == 0x86) x = 'I';
    else if (c == 0xD0 && d == 0x87) x = 'Y';
    else if (c == 0xD1 && d == 0x96) x = 'i';
    else if (c == 0xD1 && d == 0x97) x = 'y';
    else if (c == 0xD2 && (d == 0x90 || d == 0x91)) x = 'G';
    out[n++] = x;
  }
  out[n] = 0;
}

void RadarMode::begin(const Settings& s) {
  radarInit(s);
  renderedOk_ = 0xFFFFFFFF;
  renderedError_ = false;
  dirty_ = true;
}

void RadarMode::invalidate(const Settings& s) {
  radarInit(s);
  dirty_ = true;
}

void RadarMode::render(const Settings& s) {
  if (!s.radar.region[0] || !s.radar.district[0]) {
    gfxMessage("AIR ALERTS", "Choose oblast & district", C_YELLOW);
    return;
  }
  const RadarSituation& d = radarSituation();
  if (!d.valid) { gfxMessage("AIR ALERTS", radarStageName(), C_YELLOW); return; }
  Arduino_GFX* g = gfxDev();
  if (!g) return;
  const uint16_t color = d.level == 2 ? C_RED : d.level == 1 ? C_YELLOW : C_GREEN;
  const char* status = d.level == 2 ? "AIR ALERT" : d.level == 1 ? "THREATS NEARBY" : "NO THREATS";
  char place[48];
  labelAscii(d.placeName, place, sizeof(place));
  g->fillScreen(C_BLACK);
  g->fillRect(0, 0, TFT_WIDTH, 54, color);
  gfxDrawCentered(status, 16, gfxFitSize(status, 232, 3), C_BLACK);
  gfxDrawCentered(place, 66, gfxFitSize(place, 232, 2), C_WHITE);

  g->setTextColor(C_GRAY); g->setTextSize(2); g->setCursor(18, 108); g->print("DRONES");
  g->setTextColor(color);  g->setTextSize(4); g->setCursor(156, 96); g->print(d.drones);
  g->drawFastHLine(16, 145, 208, C_DGRAY);
  g->setTextColor(C_GRAY); g->setTextSize(2); g->setCursor(18, 168); g->print("ROCKETS");
  g->setTextColor(color);  g->setTextSize(4); g->setCursor(156, 156); g->print(d.missiles);
  g->drawFastHLine(16, 205, 208, C_DGRAY);
  char age[32];
  snprintf(age, sizeof(age), "updated %lus ago", (unsigned long)((millis() - radarLastOkMs()) / 1000));
  gfxDrawCentered(age, 216, 1, radarError() ? C_RED : C_GRAY);
}

void RadarMode::service(const Settings& s) {
  radarService(s);
  uint32_t ok = radarLastOkMs();
  bool err = radarError();
  if (ok != renderedOk_ || err != renderedError_) { renderedOk_ = ok; renderedError_ = err; dirty_ = true; }
  if (dirty_) { render(s); dirty_ = false; }
}
