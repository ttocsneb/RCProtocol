#include "Arduino.h"
#include "rcRemoteProtocol.h"
#include "RF24.h"

#define _ID_SIZE 5

RemoteProtocol::RemoteProtocol(RF24 *tranceiver, const uint8_t remoteId[]) {
  _radio = tranceiver;
  _remoteId = remoteId;


}

void RemoteProtocol::begin() { 
  _radio->begin();
  _radio->enableDynamicPayloads();
  _radio->setDataRate(RF24_250KBPS);

  _radio->setRetries(15, 15);
  _radio->stopListening();
}

int8_t RemoteProtocol::pair(void (*saveSettings)(uint8_t*, uint8_t*, uint8_t)) {
  _radio->stopListening();
  _radio->openWritingPipe(_PAIR_ADDRESS);

  _flushBuffer();
  
  uint8_t deviceId[_ID_SIZE];

  //Send the remote's id until a receiver has acknowledged
  if(_forceSend(const_cast<uint8_t*>(_remoteId), _ID_SIZE, RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  for(int i=0; i<5; i++) {
    Serial.write(_remoteId[i]);
  }

  //Start listening for the device to send data back
  _radio->openReadingPipe(1, _remoteId);
  _radio->startListening();

  //wait until data is available, if it takes too long, error lost connection
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;

  _radio->read(&deviceId, _ID_SIZE);

  //Read the settings from the device

  uint8_t i = 0;
  uint8_t deviceSettings[32];//32 is the maximum size of a transmission

  /*
  //Read the settings to deviceSettings, and get the size.
  while(_radio->available() && i<32) {
    _radio->read(&deviceSettings[i], 1);
    i++;
  }

  //Save settings
  */
  saveSettings(deviceId, deviceSettings, i);


  _radio->stopListening();

  return 0;
}

int8_t RemoteProtocol::connect(bool (checkIfValid)(uint8_t*)) {

  _radio->openReadingPipe(1, _remoteId); 
  _radio->startListening();
  
  if(_waitTillAvailable(RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  //Check if the required add-ons are present
  uint8_t deviceId[_ID_SIZE];

  _radio->read(&deviceId, _ID_SIZE);

  bool valid = checkIfValid(deviceId);

  //Send yes or no, if we can connect

  _radio->stopListening();
  _radio->openWritingPipe(_remoteId);


  if(valid) {
    if(_forceSend(const_cast<uint8_t*>(&_YES), 1, RC_CONNECT_TIMEOUT) != 0) 
      return RC_ERROR_LOST_CONNECTION;
  } else {
    if(_forceSend(const_cast<uint8_t*>(&_NO), 1, RC_CONNECT_TIMEOUT) != 0) 
      return RC_ERROR_LOST_CONNECTION;
  }

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

void RemoteProtocol::_flushBuffer() {
  uint8_t tmp;
  while(_radio->available()) {
    _radio->read(&tmp, 1);
  }
}
