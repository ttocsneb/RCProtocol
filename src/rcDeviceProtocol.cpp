#include <Arduino.h>
#include <RF24.h>

#include "rcDeviceProtocol.h"
#include "rcSettings.h"

#define _ID_SIZE 5

#define _PAIR_SIZE 32
#define _PAIR_CHANNEL 63

DeviceProtocol::DeviceProtocol(RF24 *tranceiver, const uint8_t deviceId[]) {
  _isConnected = false;

  for(uint8_t i = 0; i < _ID_SIZE; i++) {
    _remoteId[i] = 0;
  }

  _radio = tranceiver;
  _deviceId = deviceId;

  //Set Connection Settings
  _pairSettings.setEnableDynamicPayload(false);
  _pairSettings.setEnableAck(true);
  _pairSettings.setEnableAckPayload(false);
  _pairSettings.setDataRate(RF24_1MBPS);
  _pairSettings.setStartChannel(_PAIR_CHANNEL);
  _pairSettings.setPayloadSize(_PAIR_SIZE);
  _pairSettings.setRetryDelay(7);

}

void DeviceProtocol::begin(RCSettings* settings) {
  _settings = settings;
  
  _radio->begin();
  _radio->stopListening();
}

int8_t DeviceProtocol::pair(DeviceProtocol::saveRemoteID saveRemoteID) {
  uint8_t radioId[_ID_SIZE];
  bool sent = false;

  //Set the PA level to low as the pairing devices are going to be fairly close to each other.
  _radio->setPALevel(RF24_PA_LOW);

  _applySettings(&_pairSettings);
  
  //Don't yet open a writing pipe as we don't know who we will write to
  _radio->openReadingPipe(1, _PAIR_ADDRESS);

  _radio->startListening();

  //clear the buffer of any unread messages.
  _flushBuffer();

  //wait until data is available from the remote
  if(_waitTillAvailable(RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;
  
  //Read the Radio's ID
  _radio->read(&radioId, _ID_SIZE);

  //write to the remote the device id
  saveRemoteID(radioId);

  _radio->stopListening();

  //Now that we know who we will write to, open the writing pipe
  _radio->openWritingPipe(radioId);

  //Delay so that the remote has time to start listening
  delay(200);
  
  //Send the device id to the remote
  sent = _radio->write(const_cast<uint8_t*>(_deviceId), _ID_SIZE);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  delay(200);

  //Send the settings to the remote
  sent = _radio->write(const_cast<uint8_t*>(_settings->getSettings()), 32);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  return 0;
}

int8_t DeviceProtocol::connect(uint8_t remoteId[]) {
  uint8_t connectSuccess = 0;
  uint8_t test = 0;

  //reset connected because if we fail connecting, we will not be connected to anything.
  _isConnected = false;
  
  _radio->setPALevel(RF24_PA_LOW);

  _applySettings(&_pairSettings);

  _radio->openWritingPipe(remoteId);
  _radio->openReadingPipe(1, _deviceId);

  _flushBuffer();

  //Start writing
  _radio->stopListening();

  //send the device id to the remote, this announces who we are.
  if(_forceSend(const_cast<uint8_t*>(_deviceId), 5, RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  _radio->startListening();

  //Wait until a response is made
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;
  
  _radio->read(&connectSuccess, 1);

  _radio->stopListening();

  //check if the connection was successful
  if(connectSuccess == _NO) {
    return RC_ERROR_CONNECTION_REFUSED;
  } else if(connectSuccess != _YES) {
    return RC_ERROR_BAD_DATA;
  }

  _applySettings(_settings);

  _radio->setPALevel(RF24_PA_HIGH);

  _radio->startListening();


  if(_settings->getEnableAck()) {
    if(_settings->getEnableAckPayload()) {
      _radio->writeAckPayload(1, &_TEST, 1);

      if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0){
        _radio->stopListening();
        return RC_ERROR_LOST_CONNECTION;
      } 

      _radio->read(&test, 1);
      
      if(test != _TEST) {
        _radio->stopListening();
        return RC_ERROR_BAD_DATA;
      }

    } else {
      if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) {
        _radio->stopListening();
        return RC_ERROR_LOST_CONNECTION;
      }

      _radio->read(&test, 1);

      if(test != _TEST) {
        _radio->stopListening();
        return RC_ERROR_BAD_DATA;
      }
    }
  } else {
    if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) {
      _radio->stopListening();
      return RC_ERROR_LOST_CONNECTION;
    }

    _radio->read(&test, 1);

    if(test != _TEST) {
      _radio->stopListening();
      return RC_ERROR_BAD_DATA;
    }

    _radio->stopListening();

    delay(200);

    _radio->write(&_TEST, 1);

    _radio->startListening();

  }

  //We passed all of the tests, so we are connected.
  _isConnected = true;

  for(uint8_t i = 0; i < _ID_SIZE; i++) {
    _remoteId[i] = remoteId[i];
  }

  return 0;
}

bool DeviceProtocol::isConnected() {
  return _isConnected;
}

int8_t DeviceProtocol::_checkPacket(uint8_t *returnData) {
  if(!isConnected()) {
    return RC_ERROR_NOT_CONNECTED;
  }

  if(_radio->available()) {
    _radio->read(returnData, _settings->getPayloadSize());

    return 1;
  }

  return 0;
}

int8_t DeviceProtocol::update(uint16_t channels[], uint8_t telemetry[]) {
  if(!isConnected()) {
    return RC_ERROR_NOT_CONNECTED;
  }

  uint8_t returnData[_settings->getPayloadSize()];

  int8_t packetStatus = 0;
  int8_t status = 0;

  //read through each transmission we have gotten since the last update
  while(packetStatus == 0) {

    //Load a transmission.
    packetStatus = _checkPacket(returnData);

    //if the a packet was received
    if(packetStatus == 0) {
      //When the packet is a Standard Packet
      if(returnData[0] == _STDPACKET) {
        //copy the received data to channels
        memcpy(channels, returnData + 1, 
          min(static_cast<uint8_t>(_settings->getNumChannels() * sizeof(uint16_t)), 
            _settings->getPayloadSize() - 1));
        
        //Send the ack payload.
        if(_settings->getEnableAckPayload()) {
          _radio->writeAckPayload(1, telemetry, _settings->getPayloadSize());
        }

        status = 1;
      }
    } else if(packetStatus < 0) {
      status = packetStatus;
    }
  }

  return status;
}

int8_t DeviceProtocol::_forceSend(void *buf, uint8_t size, unsigned long timeout) {
  
  unsigned long t = millis();
  bool ack = false;
  while(!ack && millis()-t < timeout) {
    ack = _radio->write(buf, size);
  }
  if(!ack) return -1;

  return 0;
}

int8_t DeviceProtocol::_waitTillAvailable(unsigned long timeout) {
  unsigned long t = millis();
  while(!_radio->available() && millis()-t < timeout) delay(16);
  if(millis()-t >= timeout) return -1;

  return 0;
}

void DeviceProtocol::_applySettings(RCSettings *settings) {

  //Enable/disable Dynamic Payloads, and set payload size
  if(settings->getEnableDynamicPayload()) {
    _radio->enableDynamicPayloads();
  } else {
    _radio->disableDynamicPayloads();
    _radio->setPayloadSize(settings->getPayloadSize());
  }

  //Enable/Disable autoack, and custom payloads.
  _radio->setAutoAck(settings->getEnableAck());
  if(settings->getEnableAck() && settings->getEnableAckPayload()) {
    _radio->enableAckPayload();
  }

  //Set the channel
  _radio->setChannel(settings->getStartChannel());

  //Set the data rate
  _radio->setDataRate(settings->getDataRate());
  
  //Set the Retry delay.  I might add retry number as an option later.
  _radio->setRetries(settings->getRetryDelay(), 15);
}

void DeviceProtocol::_flushBuffer() {
  //read until there is no more data in the read buffer.
  uint8_t tmp;
  while(_radio->available()) {
    _radio->read(&tmp, 1);
  }
}