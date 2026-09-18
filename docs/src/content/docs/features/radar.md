---
title: Air-alert Radar
description: Regional air-alert status on the ESP8266 SmallTV.
---

Set a Ukrainian region in the **Radar** tab. Optionally provide a district and city, then select whether the screen represents that locality or the whole region.

The screen shows the selected place, green/yellow/red alert level, and drone and missile counts. It refreshes from `https://radar.syslog.pp.ua/v1/situation`; settings control the refresh interval.

If the API is temporarily unavailable, the display keeps the last successful reading and retries instead of blocking the device.
