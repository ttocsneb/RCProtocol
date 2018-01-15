#include <RF24.h>

#include "rcDeviceProtocol.h"
#include "rcSettings.h"

DeviceProtocol::DeviceProtocol(RF24* tranceiver, const uint8_t deviceId[]) {
  _isConnected = false;

  for(uint8_t i = 0; i < 5; i++) {
    _remoteId[i] = 0;
  }

  _radio = tranceiver;
  _deviceId = deviceId;

}

void DeviceProtocol::begin(RCSettings* settings) {
  _settings.setSettings(settings->getSettings());

  _radio->begin();
  _radio->stopListening();
}

int8_t DeviceProtocol::pair(DeviceProtocol::saveRemoteID saveRemoteID) {
  uint8_t radioId[5];
  bool sent = false;

  //Set the PA level to low as the pairing devices are going to be fairly
  //close to each other.
  _radio->setPALevel(RF24_PA_LOW);

  apply_settings(&_pairSettings);

  //Don't yet open a writing pipe as we don't know who we will write to
  _radio->openReadingPipe(1, _PAIR_ADDRESS);

  _radio->startListening();

  //clear the buffer of any unread messages.
  flush_buffer();

  //wait until data is available from the remote
  if(wait_till_available(RC_TIMEOUT) != 0) {
    return RC_ERROR_TIMEOUT;
  }

  //Read the Radio's ID
  _radio->read(&radioId, 5);

  //write to the remote the device id
  saveRemoteID(radioId);

  _radio->stopListening();

  //Now that we know who we will write to, open the writing pipe
  _radio->openWritingPipe(radioId);

  //Delay so that the remote has time to start listening
  delay(200);

  //Send the device id to the remote
  sent = _radio->write(const_cast<uint8_t*>(_deviceId), 5);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  delay(200);

  //Send the settings to the remote
  sent = _radio->write(const_cast<uint8_t*>(_settings.getSettings()), 32);
  if(!sent) {
    return RC_ERROR_LOST_CONNECTION;
  }

  return 0;
}

int8_t DeviceProtocol::connect(uint8_t remoteId[]) {
  uint8_t connectSuccess = 0;
  uint8_t test = 0;

  //reset connected because if we fail connecting, we will not be connected
  //to anything.
  _isConnected = false;

  _radio->setPALevel(RF24_PA_LOW);

  apply_settings(&_pairSettings);

  _radio->openWritingPipe(remoteId);
  _radio->openReadingPipe(1, _deviceId);

  flush_buffer();

  //Start writing
  _radio->stopListening();

  //send the device id to the remote, this announces who we are.
  if(force_send(const_cast<uint8_t*>(_deviceId), 5, RC_TIMEOUT) != 0) {
    return RC_ERROR_TIMEOUT;
  }

  _radio->startListening();

  //Wait until a response is made
  if(wait_till_available(RC_CONNECT_TIMEOUT) != 0) {
    return RC_ERROR_LOST_CONNECTION;
  }

  _radio->read(&connectSuccess, 1);

  _radio->stopListening();

  //check if the connection was successful
  if(connectSuccess == _NACK) {
    return RC_ERROR_CONNECTION_REFUSED;
  } else if(connectSuccess != _ACK) {
    return RC_ERROR_BAD_DATA;
  }

  apply_settings(&_settings);

  _radio->setPALevel(RF24_PA_HIGH);

  _radio->startListening();


  if(_settings.getEnableAck()) {
    if(_settings.getEnableAckPayload()) {
      _radio->writeAckPayload(1, &_TEST, 1);

      if(wait_till_available(RC_CONNECT_TIMEOUT) != 0) {
        _radio->stopListening();
        return RC_ERROR_LOST_CONNECTION;
      }

      _radio->read(&test, 1);

      if(test != _TEST) {
        _radio->stopListening();
        return RC_ERROR_BAD_DATA;
      }

    } else {
      if(wait_till_available(RC_CONNECT_TIMEOUT) != 0) {
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
    if(wait_till_available(RC_CONNECT_TIMEOUT) != 0) {
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

  for(uint8_t i = 0; i < 5; i++) {
    _remoteId[i] = remoteId[i];
  }

  return 0;
}

bool DeviceProtocol::isConnected() {
  return _isConnected;
}

int8_t DeviceProtocol::check_packet(void* returnData, uint8_t dataSize,
                                    void* telemetry, uint8_t telemetrySize) {
  if(!isConnected()) {
    return RC_ERROR_NOT_CONNECTED;
  }

  uint8_t pipe = 0;

  if(_radio->available(&pipe)) {
    _radio->read(returnData, dataSize);

    //Check if the telemetry should be sent through the ackPayload
    if(telemetry && _settings.getEnableAckPayload()) {
      _radio->writeAckPayload(pipe, telemetry, telemetrySize);
    }

    return 1;
  }

  return 0;
}

int8_t DeviceProtocol::check_packet(void* returnData, uint8_t dataSize) {
  return check_packet(returnData, dataSize, NULL, 0);
}

int8_t DeviceProtocol::update(uint16_t channels[], uint8_t telemetry[]) {
  if(!isConnected()) {
    return RC_ERROR_NOT_CONNECTED;
  }

  int8_t packetStatus = 0;
  int8_t status = 0;

  //Load a transmission, and send an ack payload.
  packetStatus = check_packet(channels,
                              _settings.getNumChannels() * sizeof(uint16_t),
                              telemetry, _settings.getPayloadSize());

  //read through each transmission we have gotten since the last update
  while(packetStatus == 1) {

    //Load a transmission.
    packetStatus = check_packet(channels,
                                _settings.getNumChannels() * sizeof(uint16_t));

    //if the a packet was received
    if(packetStatus == 1) {

      //Do stuff when a packet was received

      status = 1;
    } else if(packetStatus < 0) {
      status = packetStatus;
    }
  }

  return status;
}

RCSettings* DeviceProtocol::getSettings() {
  return &_settings;
}