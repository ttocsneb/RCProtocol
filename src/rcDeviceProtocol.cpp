#include "Arduino.h"
#include "rcDeviceProtocol.h"
#include "RF24.h"


DeviceProtocol::DeviceProtocol(RF24 *tranceiver, const uint8_t deviceId[]) {
  _radio = tranceiver;
  _deviceId = deviceId;
}

void DeviceProtocol::begin(const uint8_t* settings) {
  _settings = settings;
  
  _radio->begin();

  _radio->setRetries(15, 15);
  _radio->stopListening();
}

int8_t DeviceProtocol::pair(void (saveRemoteID)(uint8_t*)) {
  _radio->setAutoAck(true);
  _radio->setDataRate(RF24_1MBPS);
  _radio->setPayloadSize(32);
  _radio->setPALevel(RF24_PA_LOW);
  _radio->setChannel(63);

  _radio->openReadingPipe(1, _PAIR_ADDRESS);
  _radio->startListening();

  _clearBuffer();

  uint8_t radioId[5];
  bool sent = false;

  //wait until data is available from the remote
  if(_waitTillAvailable(RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;
  
  _radio->read(&radioId, 5);

  //write to the remote the device id
  saveRemoteID(radioId);

  _radio->stopListening();
  _radio->openWritingPipe(radioId);

  delay(200);
  
  //Send the device id to the remote
  sent = _radio->write(const_cast<uint8_t*>(_deviceId), 5);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  delay(200);
  //Send the settings to the remote
  sent = _radio->write(const_cast<uint8_t*>(_settings), 32);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  return 0;

}

int8_t DeviceProtocol::connect(uint8_t remoteId[]) {
  _radio->setAutoAck(true);
  _radio->setPayloadSize(32);
  _radio->setPALevel(RF24_PA_LOW);
  _radio->setChannel(63);

  _radio->stopListening();
  _radio->openWritingPipe(remoteId);

  //send the device id to the remote, this announces who we are.
  if(_forceSend(const_cast<uint8_t*>(_deviceId), 5, RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  _radio->openReadingPipe(1, remoteId);
  _radio->startListening();

  //Wait until a response is made
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;
  
  uint8_t connectSuccess = 0;

  _radio->read(&connectSuccess, 1);

  _radio->stopListening();

  //check if the connection was successful
  if(connectSuccess == _NO) {
    return RC_ERROR_CONNECTION_REFUSED;
  } else if(connectSuccess == _YES) {
    //Set the radio settings

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
    _radio->setPALevel(RF24_PA_HIGH);
    _radio->setChannel(_settings[SET_START_CHANNEL]);
    switch(_settings[SET_DATA_RATE]) {
    case RF24_2MBPS:
      _radio->setDataRate(RF24_2MBPS);
      break;
    case RF24_250KBPS:
      _radio->setDataRate(RF24_250KBPS);
      break;
    case RF24_1MBPS:
    default:
      _radio->setDataRate(RF24_1MBPS);
    }

    _radio->startListening();

    if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_CONNECTION_BAD_DATA;

    uint8_t test = 0;

    _radio->read(&test, 1);

    if(test != _TEST) return RC_ERROR_CONNECTION_BAD_DATA;
    

    return 0;
  }

  return RC_ERROR_CONNECTION_BAD_DATA;
}

int8_t DeviceProtocol::update() {
  return 0;
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

void DeviceProtocol::_clearBuffer() {
  //read until there is no more data in the read buffer.
  uint8_t tmp;
  while(_radio->available()) {
    _radio->read(&tmp, 1);
  }
}