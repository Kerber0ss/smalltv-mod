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

## Companion clients

The standard firmware can show Claude and Codex usage. It needs one client running on the Mac that owns the corresponding login.

| Service | Client | macOS setup |
|---|---|---|
| Codex | Included [`codex-meter`](codex-meter/README.md) | Run `python3 codex_meter.py`; it finds SmallTV devices over Bonjour and pushes usage directly. |
| Claude | External [clawdmeter-daemon](https://github.com/giovi321/clawdmeter-daemon) (v1.1.0+) | Clone the upstream project and run `./install.sh`; it creates a virtualenv and menu-bar LaunchAgent. |

`clawdmeter-daemon` remains an external dependency so its OAuth-token handling and macOS installer have one maintained source of truth.

## Build

```bash
pio run -e smalltv
```

The output is `.pio/build/smalltv/firmware.bin`. `smalltv_lean` and `smalltv_loader` are ESP8266 variants of the same firmware.
