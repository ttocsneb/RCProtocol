#include <RF24.h>
#include <printf.h>

#include "rcRemoteProtocol.h"
#include "rcSettings.h"


#define _ID_SIZE 5

#define _PAIR_SIZE 32
#define _PAIR_CHANNEL 63

RemoteProtocol::RemoteProtocol(RF24* tranceiver, const uint8_t remoteId[]) {
  //initialize all primitive variables
  _isConnected = false;

  for(uint8_t i = 0; i < _ID_SIZE; i++) {
    _deviceId[i] = 0;
  }

  _timer = millis();

  _radio = tranceiver;
  _remoteId = remoteId;

  //Set connection settings
  _pairSettings.setEnableDynamicPayload(false);
  _pairSettings.setEnableAck(true);
  _pairSettings.setEnableAckPayload(false);
  _pairSettings.setDataRate(RF24_1MBPS);
  _pairSettings.setStartChannel(_PAIR_CHANNEL);
  _pairSettings.setPayloadSize(_PAIR_SIZE);
  _pairSettings.setRetryDelay(7);
}

void RemoteProtocol::begin() {
  _radio->begin();

  _radio->stopListening();

}

int8_t RemoteProtocol::pair(RemoteProtocol::saveSettings saveSettings) {
  uint8_t settings[32];
  uint8_t deviceId[_ID_SIZE];

  _radio->setPALevel(RF24_PA_LOW);

  _applySettings(&_pairSettings);

  _radio->stopListening();
  _radio->openWritingPipe(_PAIR_ADDRESS);

  //clear the buffer of any unread messages.
  _flushBuffer();

  //Send the remote's id until a receiver has acknowledged
  if(_forceSend(const_cast<uint8_t*>(_remoteId), _ID_SIZE, RC_TIMEOUT) != 0) {
    return RC_ERROR_TIMEOUT;
  }

  //Start listening for the device to send data back
  _radio->openReadingPipe(1, _remoteId);
  _radio->startListening();

  //wait until data is available, if it takes too long, error lost connection
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) {
    return RC_ERROR_LOST_CONNECTION;
  }

  //read the deviceId
  _radio->read(&deviceId, _ID_SIZE);


  //wait until data is available, if it takes too long, error lost connection
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) {
    return RC_ERROR_LOST_CONNECTION;
  }

  //Read the settings to settings
  _radio->read(settings, 32);

  //Save settings
  saveSettings(deviceId, settings);

  _radio->stopListening();

  return 0;
}

int8_t RemoteProtocol::connect(RemoteProtocol::checkIfValid checkIfValid) {
  uint8_t settings[32];
  uint8_t testData = 0;
  bool valid = false;

  //reset connected because if we fail connecting, we will not be connected
  //to anything.
  _isConnected = false;

  //Set the PA level to low since the two devices will be close to eachother
  _radio->setPALevel(RF24_PA_LOW);

  _applySettings(&_pairSettings);

  //We don't yet open a writing pipe as we don't know who we will write to.
  _radio->openReadingPipe(1, _remoteId);

  _radio->startListening();

  //Wait for communications, timeout error if it takes too long
  if(_waitTillAvailable(RC_TIMEOUT) != 0) {
    return RC_ERROR_TIMEOUT;
  }

  //Read the device ID
  _radio->read(&_deviceId, _ID_SIZE);

  //Check if we can pair with the device
  valid = checkIfValid(_deviceId, settings);
  _settings.setSettings(settings);

  //Start Writing
  _radio->stopListening();

  //We now know who we will be writing to, so open the writing pipe
  _radio->openWritingPipe(_deviceId);

  //Delay so that the device has time to switch modes
  delay(200);

  //If the device is allowed to connect, send the _YES command, else _NO
  if(valid) {
    if(_radio->write(const_cast<uint8_t*>(&_YES), 1) == false) {
      return RC_ERROR_LOST_CONNECTION;
    }
  } else {
    if(_radio->write(const_cast<uint8_t*>(&_NO), 1) == false) {
      return RC_ERROR_LOST_CONNECTION;
    }
    return RC_ERROR_CONNECTION_REFUSED;
  }

  //Set the radio settings to the settings specified by the receiver.
  _applySettings(&_settings);

  _radio->setPALevel(RF24_PA_HIGH);

  delay(200);

  //Test if the settings were set correctly.

  if(_settings.getEnableAck()) {
    if(_settings.getEnableAckPayload()) {
      //Test when ack payloads are enabled.
      if(_radio->write(const_cast<uint8_t*>(&_TEST), 1) == false) {
        return RC_ERROR_LOST_CONNECTION;
      }

      if(_radio->available()) {
        _radio->read(&testData, 1);

        if(testData != _TEST) {
          return RC_ERROR_BAD_DATA;
        }
      } else {
        return RC_ERROR_BAD_DATA;
      }

    } else {

      //Test When Ack Payloads are disabled
      if(_radio->write(const_cast<uint8_t*>(&_TEST), 1) == false) {
        return RC_ERROR_BAD_DATA;
      }
    }
  } else {
    //Test when acks are disabled
    _radio->write(&_TEST, 1);

    _radio->startListening();

    if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) {
      return RC_ERROR_LOST_CONNECTION;
    }

    _radio->read(&testData, 1);

    if(testData != _TEST) {
      return RC_ERROR_BAD_DATA;
    }

    _radio->stopListening();
  }

  //We passed all of the tests, so we are connected.
  _isConnected = true;
  //set timer delay as a variable once so it doesn't need to be recalculated
  //every update
  _timerDelay = round(1000.0 / _settings.getCommsFrequency());

  return 0;
}

bool RemoteProtocol::isConnected() {
  return _isConnected;
}

int8_t RemoteProtocol::_sendPacket(void* data, uint8_t dataSize,
                                   void* telemetry, uint8_t telemetrySize) {
  if(isConnected()) {

    //send data
    if(_radio->write(data, dataSize)) {

      //Check if a payload was sent back.
      if(_radio->isAckPayloadAvailable()) {
        //set telemetry to whatever was sent back
        _radio->read(telemetry, telemetrySize);
        return 1;
      }
    } else if(_settings.getEnableAck()) {
      //We were expecting at least an ack, but did not get one
      return RC_ERROR_PACKET_NOT_SENT;
    }

    return 0;
  } else {
    return RC_ERROR_NOT_CONNECTED;
  }
}

int8_t RemoteProtocol::update(uint16_t channels[], uint8_t telemetry[]) {

  //const uint8_t PACKET_BEGIN = 1;


  if(!isConnected()) {
    return RC_ERROR_NOT_CONNECTED;
  }

  //Send the packet.
  int8_t status = _sendPacket(channels,
                              sizeof(uint16_t) * _settings.getNumChannels(),
                              telemetry, sizeof(uint8_t) * _settings.getPayloadSize());


  //Check if the frequency delay has already passed.
  if(millis() - _timer > _timerDelay && status >= 0) {
    status = RC_INFO_TICK_TOO_SHORT;
  }
  //wait until The Comms Frequency delay has passed.
  while(millis() - _timer < _timerDelay) {
    delay(1);
  }

  _timer = millis();

  return status;
}

int8_t RemoteProtocol::_forceSend(void* buf, uint8_t size, uint32_t timeout) {

  uint32_t t = millis();
  bool ack = false;
  while(!ack && millis() - t < timeout) {
    ack = _radio->write(buf, size);
  }
  if(!ack) {
    return -1;
  }

  return 0;
}

int8_t RemoteProtocol::_waitTillAvailable(uint32_t timeout) {
  uint32_t t = millis();
  while(!_radio->available() && millis() - t < timeout) {
    delay(16);
  }
  if(millis() - t >= timeout) {
    return -1;
  }

  return 0;
}

void RemoteProtocol::_applySettings(RCSettings* settings) {

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

void RemoteProtocol::_flushBuffer() {
  uint8_t tmp;
  while(_radio->available()) {
    _radio->read(&tmp, 1);
  }
}
