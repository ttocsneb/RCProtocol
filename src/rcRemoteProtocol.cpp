#include "Arduino.h"
#include "rcRemoteProtocol.h"
#include "RF24.h"

#define _ID_SIZE 5

#define _PAIR_SIZE 32
#define _PAIR_CHANNEL 63

RemoteProtocol::RemoteProtocol(RF24 *tranceiver, const uint8_t remoteId[]) {
  _radio = tranceiver;
  _remoteId = remoteId;
}

void RemoteProtocol::begin() { 
  _radio->begin();

  _radio->setRetries(15, 15);
  _radio->stopListening();
}

int8_t RemoteProtocol::pair(RemoteProtocol::saveSettings saveSettings) {
  uint8_t settings[32];
  uint8_t deviceId[_ID_SIZE];

  //Set Pair settings
  _radio->disableDynamicPayloads();
  _radio->setAutoAck(true);
  _radio->setDataRate(RF24_1MBPS);
  _radio->setPayloadSize(_PAIR_SIZE);
  _radio->setPALevel(RF24_PA_LOW);
  _radio->setChannel(_PAIR_CHANNEL);

  _radio->stopListening();
  _radio->openWritingPipe(_PAIR_ADDRESS);

  //clear the buffer of any unread messages.
  _flushBuffer();
  
  //Send the remote's id until a receiver has acknowledged
  if(_forceSend(const_cast<uint8_t*>(_remoteId), _ID_SIZE, RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  //Start listening for the device to send data back
  _radio->openReadingPipe(1, _remoteId);
  _radio->startListening();

  //wait until data is available, if it takes too long, error lost connection
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;

  //read the deviceId
  _radio->read(&deviceId, _ID_SIZE);

  
  //wait until data is available, if it takes too long, error lost connection
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;

  //Read the settings to settings
  _radio->read(settings, 32);

  //Save settings  
  saveSettings(deviceId, settings);

  _radio->stopListening();

  return 0;
}

int8_t RemoteProtocol::connect(RemoteProtocol::checkIfValid checkIfValid) {
  uint8_t deviceId[_ID_SIZE];
  uint8_t settings[32];
  bool valid = false;

  //Set Connect settings
  _radio->disableDynamicPayloads();
  _radio->setAutoAck(true);
  _radio->setDataRate(RF24_1MBPS);
  _radio->setPayloadSize(_PAIR_SIZE);
  _radio->setPALevel(RF24_PA_LOW);
  _radio->setChannel(_PAIR_CHANNEL);

  _flushBuffer();

  //Start Listening
  _radio->openReadingPipe(1, _remoteId); 
  _radio->startListening();
  
  //Wait for communications, timeout error if it takes too long
  if(_waitTillAvailable(RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  //Read the device ID
  _radio->read(&deviceId, _ID_SIZE);

  //Check if we can pair with the device
  valid = checkIfValid(deviceId, settings);
  _settings.setSettings(settings);

  //Start Writing
  _radio->stopListening();
  _radio->openWritingPipe(_remoteId);

  //Delay so that the device has time to switch modes
  delay(200);

  //If the device is allowed to connect, send the _YES command, else _NO
  if(valid) {
    if(_radio->write(const_cast<uint8_t*>(&_YES), 1) == false) 
      return RC_ERROR_LOST_CONNECTION;
  } else {
    if(_radio->write(const_cast<uint8_t*>(&_NO), 1) == false) 
      return RC_ERROR_LOST_CONNECTION;
    return RC_ERROR_CONNECTION_REFUSED;
  }
  
  //Set the radio settings
  _applySettings();
  
  //Delay so that the device has time to apply their settings
  delay(200);

  //Test if the settings were set correctly.  
  //NOTE: I haven't tested what would happen if auto-ack is disabled.
  if(_forceSend(const_cast<uint8_t*>(&_TEST), 1, RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_BAD_DATA;

  return 0;
}

int8_t RemoteProtocol::update() {
  return 0;
}

int8_t RemoteProtocol::_forceSend(void *buf, uint8_t size, uint32_t timeout) {
  
  uint32_t t = millis();
  bool ack = false;
  while(!ack && millis()-t < timeout) {
    ack = _radio->write(buf, size);
  }
  if(!ack) return -1;

  return 0;
}


int8_t RemoteProtocol::_waitTillAvailable(uint32_t timeout) {
  uint32_t t = millis();
  while(!_radio->available() && millis()-t < timeout) delay(16);
  if(millis()-t >= timeout) return -1;

  return 0;
}

void RemoteProtocol::_applySettings() {
  _radio->setPALevel(RF24_PA_HIGH);

  //Enable/disable Dynamic Payloads, and set payload size
  if(_settings.getEnableDynamicPayload()) {
    _radio->enableDynamicPayloads();
  } else {
    _radio->disableDynamicPayloads();
    _radio->setPayloadSize(_settings.getPayloadSize());
  }

  //Enable/Disable autoack, and custom payloads.
  _radio->setAutoAck(_settings.getEnableAck());
  if(_settings.getEnableAck() && _settings.getEnableAckPayload()) {
    _radio->enableAckPayload();
  }

  //Set the channel
  _radio->setChannel(_settings.getStartChannel());

  //Set the data rate
  _radio->setDataRate(_settings.getDataRate());
}

void RemoteProtocol::_flushBuffer() {
  uint8_t tmp;
  while(_radio->available()) {
    _radio->read(&tmp, 1);
  }
}
