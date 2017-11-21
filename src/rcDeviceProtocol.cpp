#include "Arduino.h"
#include "rcDeviceProtocol.h"
#include "RF24.h"

#define _ID_SIZE 5

#define _PAIR_SIZE 32
#define _PAIR_CHANNEL 63

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

int8_t DeviceProtocol::pair(DeviceProtocol::saveRemoteID saveRemoteID) {
  uint8_t radioId[_ID_SIZE];
  bool sent = false;

  //Set Pair settings
  _radio->disableDynamicPayloads();
  _radio->setAutoAck(true);
  _radio->setDataRate(RF24_1MBPS);
  _radio->setPayloadSize(_PAIR_SIZE);
  _radio->setPALevel(RF24_PA_LOW);
  _radio->setChannel(_PAIR_CHANNEL);

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
  _radio->openWritingPipe(radioId);

  //Delay so that the remote has time to start listening
  delay(200);
  
  //Send the device id to the remote
  sent = _radio->write(const_cast<uint8_t*>(_deviceId), _ID_SIZE);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  //Delay so that the remote has time to process the previus transmission
  delay(200);

  //Send the settings to the remote
  sent = _radio->write(const_cast<uint8_t*>(_settings), 32);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  return 0;
}

int8_t DeviceProtocol::connect(uint8_t remoteId[]) {
  uint8_t connectSuccess = 0;
  uint8_t test = 0;

  //Set Connect settings
  _radio->disableDynamicPayloads();
  _radio->setAutoAck(true);
  _radio->setDataRate(RF24_1MBPS);
  _radio->setPayloadSize(_PAIR_SIZE);
  _radio->setPALevel(RF24_PA_LOW);
  _radio->setChannel(_PAIR_CHANNEL);

  _flushBuffer();

  //Start writing
  _radio->stopListening();
  _radio->openWritingPipe(remoteId);

  //send the device id to the remote, this announces who we are.
  if(_forceSend(const_cast<uint8_t*>(_deviceId), 5, RC_TIMEOUT) != 0) return RC_ERROR_TIMEOUT;

  //Start listening
  _radio->openReadingPipe(1, remoteId);
  _radio->startListening();

  //Wait until a response is made
  if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_LOST_CONNECTION;
  
  _radio->read(&connectSuccess, 1);

  _radio->stopListening();

  //check if the connection was successful
  if(connectSuccess == _NO) {
    return RC_ERROR_CONNECTION_REFUSED;
  } else if(connectSuccess == _YES) {
    //Set the radio settings

    _applySettings();

    _radio->startListening();

    if(_waitTillAvailable(RC_CONNECT_TIMEOUT) != 0) return RC_ERROR_BAD_DATA;


    _radio->read(&test, 1);

    if(test != _TEST) return RC_ERROR_BAD_DATA;
    

    return 0;
  }

  return RC_ERROR_BAD_DATA;
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

void DeviceProtocol::_applySettings() {
  _radio->setPALevel(RF24_PA_HIGH);

  //Enable/disable Dynamic Payloads, and set payload size
  if(GET_ENABLE_DYNAMIC_PAYLOAD(_settings[SET_BOOLS]) == true) {
    _radio->enableDynamicPayloads();
  } else {
    _radio->disableDynamicPayloads();
    _radio->setPayloadSize(_settings[SET_PAYLOAD_SIZE]);
  }

  //Enable/Disable autoack, and custom payloads.
  _radio->setAutoAck(GET_ENABLE_ACK(_settings[SET_BOOLS]));
  if(GET_ENABLE_ACK(_settings[SET_BOOLS]) && GET_ENABLE_ACK_PAYLOAD(_settings[SET_BOOLS])) {
    _radio->enableAckPayload();
  }

  //Set the channel
  _radio->setChannel(_settings[SET_START_CHANNEL]);

  //Set the data rate
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
}

void DeviceProtocol::_flushBuffer() {
  //read until there is no more data in the read buffer.
  uint8_t tmp;
  while(_radio->available()) {
    _radio->read(&tmp, 1);
  }
}