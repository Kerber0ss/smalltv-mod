---
title: Which release file to download
description: ESP8266 SmallTV release files.
---

| File | Use |
|---|---|
| `smalltv-mod-firmware.bin` | Normal ESP8266 firmware. |
| `smalltv-mod-firmware-lean.bin` | ESP8266 firmware without Home Assistant screens or usage mode. |
| `smalltv-mod-loader.bin` | One-time first installer for SmallTV-ultra. |

The System tab reports `esp8266` or `esp8266-lean`; self-update keeps that variant. Switching between normal and lean is a manual upload in the System tab and keeps WiFi settings.
