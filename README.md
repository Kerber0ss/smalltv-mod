# smalltv-mod

Custom firmware for the GeekMagic SmallTV with an ESP-12F / ESP8266 and a 1.54-inch 240x240 ST7789 display. It provides Ukraine air-alert Radar, Claude/Codex usage display, Home Assistant MQTT screens, notifications, and a browser-based setup page.

## Firmware files

| File | Use |
|---|---|
| `smalltv-mod-firmware.bin` | Normal ESP8266 install and OTA update. |
| `smalltv-mod-firmware-lean.bin` | Same board, without Home Assistant screens and the usage display, for more heap. |
| `smalltv-mod-loader.bin` | One-time loader for SmallTV-ultra stock firmware whose OTA slot is too small. |

## Install

1. On the stock device, open `http://<device-ip>/update`.
2. Upload `smalltv-mod-firmware.bin`.
3. Join `SmallTV-Setup`, then open `http://192.168.4.1` and save your WiFi settings.

SmallTV-ultra uses the loader file first, then uploads the normal firmware at the loader page.

## Build

```bash
pio run -e smalltv
```

The output is `.pio/build/smalltv/firmware.bin`. `smalltv_lean` and `smalltv_loader` are ESP8266 variants of the same firmware.
