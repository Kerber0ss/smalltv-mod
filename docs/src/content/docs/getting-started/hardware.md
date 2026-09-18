---
title: Hardware
description: The supported GeekMagic SmallTV hardware and pin map.
---

smalltv-mod supports the GeekMagic SmallTV with an ESP-12F / ESP8266, 4 MB flash, and a 1.54-inch 240x240 ST7789 IPS panel.

![SmallTV ESP8266](/smalltv-mod/assets/product-8266.png)

| Signal | GPIO | Note |
|---|---:|---|
| SPI CLK | 14 | Hardware SPI |
| SPI MOSI | 13 | Hardware SPI |
| DC | 0 | Boot-strap pin |
| RST | 2 | Boot-strap pin |
| CS | 15 | Boot-strap pin |
| Backlight | 5 | PWM, active-low |

Some SmallTV-ultra units use the same ESP-12F board but have a smaller stock OTA slot. They need the loader on first install; after that they run the normal ESP8266 firmware.
