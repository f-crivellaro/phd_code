/****************************************************************************************************************************
  PublishPacket.cpp
  
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
 
#include "PublishPacket.hpp"

using AsyncMqttClientInternals::PublishPacket;

PublishPacket::PublishPacket(ParsingInformation* parsingInformation, OnMessageInternalCallback dataCallback, OnPublishInternalCallback completeCallback)
: _parsingInformation(parsingInformation)
, _dataCallback(dataCallback)
, _completeCallback(completeCallback)
, _dup(false)
, _qos(0)
, _retain(0)
, _bytePosition(0)
, _topicLengthMsb(0)
, _topicLength(0)
, _ignore(false)
, _packetIdMsb(0)
, _packetId(0)
, _payloadLength(0)
, _payloadBytesRead(0) 
{
    _dup = _parsingInformation->packetFlags & HeaderFlag.PUBLISH_DUP;
    _retain = _parsingInformation->packetFlags & HeaderFlag.PUBLISH_RETAIN;
    char qosMasked = _parsingInformation->packetFlags & 0x06;
    
    switch (qosMasked) 
    {
      case HeaderFlag.PUBLISH_QOS0:
        _qos = 0;
        break;
      case HeaderFlag.PUBLISH_QOS1:
        _qos = 1;
        break;
      case HeaderFlag.PUBLISH_QOS2:
        _qos = 2;
        break;
    }
}

PublishPacket::~PublishPacket() 
{
}

void PublishPacket::parseVariableHeader(char* data, size_t len, size_t* currentBytePosition) 
{
  (void)len;
  
  char currentByte = data[(*currentBytePosition)++];
  
  if (_bytePosition == 0) 
  {
    _topicLengthMsb = currentByte;
  } 
  else if (_bytePosition == 1) 
  {
    _topicLength = currentByte | _topicLengthMsb << 8;
    
    if (_topicLength > _parsingInformation->maxTopicLength) 
    {
      _ignore = true;
    } 
    else 
    {
      _parsingInformation->topicBuffer[_topicLength] = '\0';
    }
  } 
  else if (_bytePosition >= 2 && _bytePosition < 2 + _topicLength) 
  {
    // Starting from here, _ignore might be true
    if (!_ignore) 
      _parsingInformation->topicBuffer[_bytePosition - 2] = currentByte;
      
    if (_bytePosition == 2 + _topicLength - 1 && _qos == 0) 
    {
      _preparePayloadHandling(_parsingInformation->remainingLength - (_bytePosition + 1));
      return;
    }
  } 
  else if (_bytePosition == 2 + _topicLength) 
  {
    _packetIdMsb = currentByte;
  } 
  else 
  {
    _packetId = currentByte | _packetIdMsb << 8;
    _preparePayloadHandling(_parsingInformation->remainingLength - (_bytePosition + 1));
  }
  _bytePosition++;
}

void PublishPacket::_preparePayloadHandling(uint32_t payloadLength) 
{
  _payloadLength = payloadLength;
  
  if (payloadLength == 0) 
  {
    _parsingInformation->bufferState = BufferState::NONE;
    
    if (!_ignore) 
    {
      _dataCallback(_parsingInformation->topicBuffer, nullptr, _qos, _dup, _retain, 0, 0, 0, _packetId);
      _completeCallback(_packetId, _qos);
    }
  } 
  else 
  {
    _parsingInformation->bufferState = BufferState::PAYLOAD;
  }
}

void PublishPacket::parsePayload(char* data, size_t len, size_t* currentBytePosition) 
{
  size_t remainToRead = len - (*currentBytePosition);
  
  if (_payloadBytesRead + remainToRead > 
    _payloadLength) remainToRead = _payloadLength - _payloadBytesRead;

  if (!_ignore) 
    _dataCallback(_parsingInformation->topicBuffer, data + (*currentBytePosition), 
                  _qos, _dup, _retain, remainToRead, _payloadBytesRead, _payloadLength, _packetId);
                  
  _payloadBytesRead += remainToRead;
  (*currentBytePosition) += remainToRead;

  if (_payloadBytesRead == _payloadLength) 
  {
    _parsingInformation->bufferState = BufferState::NONE;
    
    if (!_ignore) 
      _completeCallback(_packetId, _qos);
  }
}
