/****************************************************************************************************************************
  OutPacket.hpp
  
  AsyncMqttClient_Generic is a library for ESP32, ESP8266, Protenta_H7, STM32F7, etc. with current AsyncTCP support
  
  Based on and modified from :
  
  1) async-mqtt-client (https://github.com/marvinroger/async-mqtt-client)
  
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncMqttClient_Generic
 
  Version: 1.5.0
  
  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.0.0    K Hoang     10/03/2022 Initial coding to support only ESP32 (with SSL) and ESP8266 (without SSL)
  1.0.1    K Hoang     10/03/2022 Fix Library Manager warnings
  1.1.0    K Hoang     11/03/2022 Add support to WT32_ETH01 (with or without TLS/SSL)
  1.2.0    K Hoang     15/03/2022 Add support to STM32 using LAN8742A (without TLS/SSL)
  1.2.1    K Hoang     16/03/2022 Add support to STM32 using LAN8720 (without TLS/SSL)
  1.3.0    K Hoang     16/03/2022 Add support to Portenta_H7 using built-in Ethernet or Murata WiFi (without TLS/SSL)
  1.4.0    K Hoang     17/03/2022 Add support to Teensy 4.1 using QNEthernet Library
  1.5.0    K Hoang     14/04/2022 Add support to ESP8266 W5x00/ENC28J60 using lwip_W5100/lwip_W5500 or lwip_enc28j60 library
 *****************************************************************************************************************************/

#pragma once

#ifndef OUT_PACKET_HPP
#define OUT_PACKET_HPP

#include <stdint.h>  // uint*_t
#include <stddef.h>  // size_t
#include <algorithm>  // std::min

#include "../../Flags.hpp"

namespace AsyncMqttClientInternals 
{
  class OutPacket 
  {
    public:
      OutPacket();
      virtual ~OutPacket();
      virtual const uint8_t* data(size_t index = 0) const = 0;
      virtual size_t size() const = 0;
      bool released() const;
      uint8_t packetType() const;
      uint16_t packetId() const;
      uint8_t qos() const;
      void release();

    public:
      OutPacket* next;
      uint32_t timeout;
      uint8_t noTries;

    protected:
      static uint16_t _getNextPacketId();
      bool _released;
      uint16_t _packetId;

    private:
      static uint16_t _nextPacketId;  
  };
}  // namespace AsyncMqttClientInternals

#endif    // OUT_PACKET_HPP
