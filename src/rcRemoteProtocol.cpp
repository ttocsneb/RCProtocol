/*
 vim: softtabstop=0
 vim: expandtab
 vim: shiftwidth=2
 vim: smarttab
*/


#include "Arduino.h"
#include "rcRemoteProtocol.h"
#include "RF24.h"

RemoteProtocol::RemoteProtocol(RF24 *tranceiver, uint8_t remoteId[]) {
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
  
  uint8_t deviceId[5];

  //Send the remote's id until a receiver has acknowledged
  if(_forceSend(&_remoteId, sizeof(_remoteId), RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  //Start listening for the device to send data back
  _radio->openReadingPipe(1, _remoteId);
  _radio->startListening();

  //wait until data is available, if it takes too long, error lost connection
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;

  _radio->read(&deviceId, sizeof(deviceId));

  //Read the settings from the device

  uint8_t i = 0;
  uint8_t deviceSettings[32];//32 is the maximum size of a transmission

  //Read the settings to deviceSettings, and get the size.
  while(_radio->available() && i<32) {
    _radio->read(&deviceSettings[i], sizeof(deviceSettings[i]));
    i++;
  }

  //Save settings
  saveSettings(deviceId, deviceSettings, i); 
  
  _radio->stopListening();

  return 0;
}

int8_t RemoteProtocol::connect(bool (checkIfValid)(uint8_t*)) {

  _radio->openReadingPipe(1, _remoteId); 
  _radio->startListening();
  
  long t = millis();
  while(!_radio->available() && millis()-t < RC_TIMEOUT) delay(16);
  if(millis()-t >= RC_TIMEOUT) return RC_ERROR_TIMEOUT;

  //Check if the required add-ons are present
  uint8_t deviceId[5];
  _radio->read(&deviceId, sizeof(deviceId));

  bool valid = checkIfValid(deviceId);

  //Send yes or no, if we can connect

  _radio->stopListening();
  _radio->openWritingPipe(_remoteId);


  if(valid) {
    if(_forceSend(&_YES, sizeof(_YES), RC_CONNECT_TIMEOUT) != 0) 
      return RC_ERROR_LOST_CONNECTION;
  } else {
    if(_forceSend(&_YES, sizeof(_NO), RC_CONNECT_TIMEOUT) != 0) 
      return RC_ERROR_LOST_CONNECTION;
  }

  return 0;
}

int8_t RemoteProtocol::_forceSend(void *buf, uint8_t size, long timeout) {
  
  long t = millis();
  bool ack = false;
  while(!ack && millis()-t < timeout) {
    ack = _radio->write(&buf, size);
  }
  if(!ack) return -1;

  return 0;
}


int8_t RemoteProtocol::_waitTillAvailable(long timeout) {
  long t = millis();
  while(!_radio->available() && millis()-t < timeout) delay(16);
  if(millis()-t >= timeout) return -1;

  return 0;
}
