---
title: Recovery and credits
description: ESP8266 recovery notes and project credits.
---

Factory reset in the System tab wipes settings and returns to SETUP MODE. A failed OTA leaves the running firmware intact; retry with a manual firmware upload in the System tab.

The device uses 2.4 GHz WiFi and a small ESP8266 heap. Prefer plain HTTP for LAN webhooks when TLS becomes unreliable.

Credits: GeekMagic SmallTV, Arduino_GFX, ArduinoJson, PubSubClient, adsb.lol, clawdmeter, and claudepix.
