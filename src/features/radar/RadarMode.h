// RadarMode.h — readable 240px air-alert screen.
#pragma once
#include "Mode.h"

class RadarMode : public DisplayMode {
 public:
  const char* id() const override { return "radar"; }
  uint8_t modeConst() const override { return MODE_RADAR; }
  void begin(const Settings& s) override;
  void service(const Settings& s) override;
  void invalidate(const Settings& s) override;
  void wake(const Settings& s) override { dirty_ = true; }

 private:
  void render(const Settings& s);
  uint32_t renderedOk_ = 0xFFFFFFFF;
  bool renderedError_ = false;
  bool dirty_ = true;
};

extern RadarMode g_radarMode;
