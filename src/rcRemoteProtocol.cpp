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

  _radio->setRetries(15, 15);
  _radio->stopListening();
}

int8_t RemoteProtocol::pair(RemoteProtocol::saveSettings saveSettings) {
  _radio->disableDynamicPayloads();
  _radio->setAutoAck(true);
  _radio->setDataRate(RF24_1MBPS);
  _radio->setPayloadSize(32);
  _radio->setPALevel(RF24_PA_MIN);
  _radio->setChannel(63);


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

  uint8_t settings[32];
  
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
  _radio->disableDynamicPayloads();
  _radio->setAutoAck(true);
  _radio->setDataRate(RF24_1MBPS);
  _radio->setPayloadSize(32);
  _radio->setPALevel(RF24_PA_MIN);
  _radio->setChannel(63);

  _radio->openReadingPipe(1, _remoteId); 
  _radio->startListening();
  
  if(_waitTillAvailable(RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  //Check if the required add-ons are present
  uint8_t deviceId[_ID_SIZE];

  _radio->read(&deviceId, _ID_SIZE);

  bool valid = checkIfValid(deviceId, _settings);

  //Send yes or no, if we can connect

  _radio->stopListening();
  _radio->openWritingPipe(_remoteId);

  delay(200);

  if(valid) {
    if(_radio->write(const_cast<uint8_t*>(&_YES), 1) == false) 
      return RC_ERROR_LOST_CONNECTION;
  } else {
    if(_radio->write(const_cast<uint8_t*>(&_NO), 1) == false) 
      return RC_ERROR_LOST_CONNECTION;
  }
  
  //Set the radio settings

  _radio->setPALevel(RF24_PA_HIGH);

  if(GET_ENABLE_DYNAMIC_PAYLOAD(_settings[SET_BOOLS]) == true) {
    _radio->enableDynamicPayloads();
  } else {
    _radio->disableDynamicPayloads();
    _radio->setPayloadSize(_settings[SET_PAYLOAD_SIZE]);
  }

  _radio->setAutoAck(GET_ENABLE_ACK(_settings[SET_BOOLS]));
  if(GET_ENABLE_ACK(_settings[SET_BOOLS]) && GET_ENABLE_ACK_PAYLOAD(_settings[SET_BOOLS])) {
    _radio->enableAckPayload();
  }

  _radio->setChannel(_settings[SET_START_CHANNEL]);

  switch(_settings[SET_DATA_RATE]) {
  case RF24_250KBPS:
    _radio->setDataRate(RF24_250KBPS);
    break;
  case RF24_2MBPS:
    _radio->setDataRate(RF24_2MBPS);
    break;
  case RF24_1MBPS:
  default:
    _radio->setDataRate(RF24_1MBPS);
  }

  delay(200);

  if(_forceSend(const_cast<uint8_t*>(&_TEST), 1, RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_BAD_DATA;

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
