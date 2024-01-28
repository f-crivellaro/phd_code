# AsyncMQTT_Generic client for ESP8266, ESP32, etc.

[![arduino-library-badge](https://www.ardu-badge.com/badge/AsyncMQTT_Generic.svg?)](https://www.ardu-badge.com/AsyncMQTT_Generic)
[![GitHub release](https://img.shields.io/github/release/khoih-prog/AsyncMQTT_Generic.svg)](https://github.com/khoih-prog/AsyncMQTT_Generic/releases)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](#Contributing)
[![GitHub issues](https://img.shields.io/github/issues/khoih-prog/AsyncMQTT_Generic.svg)](http://github.com/khoih-prog/AsyncMQTT_Generic/issues)

---
---

## Table of Contents

* [Changelog](#changelog)
	* [Releases v1.5.0](#releases-v150)
	* [Releases v1.4.0](#releases-v140)
	* [Releases v1.3.0](#releases-v130)
	* [Releases v1.2.1](#releases-v121)
	* [Releases v1.2.0](#releases-v120)
	* [Releases v1.1.0](#releases-v110)
	* [Releases v1.0.1](#releases-v101)
  * [Initial Releases v1.0.0](#Initial-Releases-v100)

---
---

## Changelog

### Releases v1.5.0

1. Add support to ESP8266 W5x00 using [lwIP_w5100](https://github.com/esp8266/Arduino/tree/master/libraries/lwIP_w5100) or [lwIP_w5500](https://github.com/esp8266/Arduino/tree/master/libraries/lwIP_w5500) library
2. Add support to ESP8266 ENC28J60 using [lwIP_enc28j60](https://github.com/esp8266/Arduino/tree/master/libraries/lwIP_enc28j60) library
3. Add example [FullyFeatured_ESP8266_Ethernet](examples/ESP8266/FullyFeatured_ESP8266_Ethernet)
4. Update `Packages' Patches`

### Releases v1.4.0

1. Add support to **Teensy 4.1 using QNEthernet Library**
2. Add example for `QNEthernet`

### Releases v1.3.0

1. Add support to **Portenta_H7**, using either `Murata WiFi` or `Vision-shield Ethernet`
2. Add examples for `Portenta_H7_Ethernet` and `Portenta_H7_WiFi`

### Releases v1.2.1

1. Add support to many **STM32F4 and STM32F7 (without TLS/SSL)** using `LAN8720` Ethernet, such as F407xx, NUCLEO_F429ZI, DISCO_F746NG, NUCLEO_F746ZG, NUCLEO_F756ZG, etc.
2. Add examples for `STM32_LAN8720`

### Releases v1.2.0

1. Add support to **STM32F/L/H/G/WB/MP1 (without TLS/SSL)** using `built-in LAN8742A` Ethernet, such as Nucleo-144, DISCOVERY, etc.
2. Add examples for `STM32`

### Releases v1.1.0

1. Add support to **WT32_ETH01 (SSL and non-SSL)**
2. Add examples for `WT32_ETH01`


### Releases v1.0.1

1. Fix Library Manager warnings
2. Suppress all compiler warnings
3. Optimize library code by using `reference-passing` instead of `value-passing`

### Initial Releases v1.0.0

1. Initial porting and coding to support **ESP32 (SSL and non-SSL) and ESP8266 (non-SSL)**
