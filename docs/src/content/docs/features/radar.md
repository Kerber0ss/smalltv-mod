---
title: Air-alert Radar
description: Regional air-alert status on the ESP8266 SmallTV.
---

Choose an oblast and district from the fixed lists in the **Radar** tab. Both are required. SmallTV requests data from `radar.syslog.pp.ua` with both filters; the firmware never calls NEPTUN directly.

The service returns the oblast and selected-district levels separately. The screen reads only the selected district’s `green`, `yellow`, or `red` level and its target counts; oblast data and other districts do not affect the display. The only counters shown are drones and missiles. Other target types can be included in the district data and affect its NEPTUN level, but are not shown as separate counters. Each target record counts once; `sourceCount` is confirmations, not the number of targets.

The service fetches NEPTUN data and refreshes its cached snapshot every 10 seconds. The device’s setting controls how often it requests the filtered result.

If the API is temporarily unavailable, the display keeps the last successful reading and retries instead of blocking the device.
