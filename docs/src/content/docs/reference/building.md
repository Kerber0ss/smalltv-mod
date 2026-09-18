---
title: Building from source
description: Build the ESP8266 SmallTV firmware with PlatformIO.
---

```bash
pio run -e smalltv
pio run -e smalltv_lean
pio run -e smalltv_loader
```

`smalltv` is the normal ESP8266 image. `smalltv_lean` removes Home Assistant screens and the usage meter to make more heap available for TLS. `smalltv_loader` is the one-time OTA loader for Ultra units.

All shared firmware code targets the ESP8266 Arduino core. Board wiring is in `src/board_esp8266.h`, and `src/Platform.h` contains the ESP8266 SDK shims.
