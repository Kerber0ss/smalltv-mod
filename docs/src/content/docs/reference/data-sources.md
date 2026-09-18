---
title: Data sources
description: Ticker and radar sources used by the ESP8266 SmallTV.
---

Ticker symbols use Yahoo Finance by default. A symbol can instead use cash.ch, a GitHub-hosted JSON feed, or a LAN webhook. HTTPS on the ESP8266 is memory-sensitive; the GitHub and webhook sources are useful fallbacks for cash.ch instruments.

Radar uses adsb.lol by default. A LAN webhook can return the same `{"ac":[...]}` aircraft shape after filtering a feed server-side. adsb.fi is offered for compatibility but its Cloudflare TLS records are too large for reliable ESP8266 use.
