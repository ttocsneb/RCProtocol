/*
 vim: tabstop=2
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

int8_t RemoteProtocol::pair() {
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

  //TODO Read settings from device
  
  
  _radio->stopListening();

  return 0;
}

int8_t RemoteProtocol::connect() {

  _radio->openReadingPipe(1, _remoteId); 
  _radio->startListening();
  
  long t = millis();
  while(!_radio->available() && millis()-t < RC_TIMEOUT) delay(16);
  if(millis()-t >= RC_TIMEOUT) return RC_ERROR_TIMEOUT;

  //TODO check if the required add-ons are present

  _radio->stopListening();
  _radio->openWritingPipe(_remoteId);


  //Send yes for now.
  if(_forceSend(&_YES, sizeof(_YES), RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;




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
