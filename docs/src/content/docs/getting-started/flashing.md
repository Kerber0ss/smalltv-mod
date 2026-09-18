---
title: Flashing
description: Install smalltv-mod on the ESP8266 SmallTV.
---

## Normal SmallTV

1. Find the stock device IP address.
2. Open `http://<device-ip>/update`.
3. Upload `smalltv-mod-firmware.bin`.
4. Join the `SmallTV-Setup` hotspot and open `http://192.168.4.1`.
5. Save a 2.4 GHz WiFi network.

## SmallTV-ultra

The Ultra uses the same ESP8266 hardware but its stock OTA slot is smaller.

1. Upload `smalltv-mod-loader.bin` at the stock `/update` page.
2. Join `SmallTV-Loader` and open `http://192.168.4.1/update`.
3. Upload `smalltv-mod-firmware.bin`.

After installation, manual updates are uploaded in the firmware's System tab. GitHub self-updates queue a download at boot, so the device restarts twice.
