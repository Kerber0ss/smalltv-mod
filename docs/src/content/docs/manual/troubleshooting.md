---
title: Troubleshooting
description: Common ESP8266 SmallTV problems and recovery paths.
---

## The screen is blank

Confirm the device has power, then open its settings page and check brightness, active-low backlight, orientation, and panel colour settings.

## WiFi does not connect

Use a 2.4 GHz network. If the saved networks cannot be joined, connect to `SmallTV-Setup` and save the credentials again.

## Radar is empty

The ESP8266 has limited heap for HTTPS. Check the Status tab for free heap and the Radar diagnostic, then verify that the selected region matches the API's region key.

## Update failed

Manual OTA is always available in the System tab. Firmware 2.6.1 and older need one manual update before GitHub self-update works.
