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

int8_t DeviceProtocol::begin(RCSettings* settings,
                             DeviceProtocol::checkConnected checkConnected,
                             DeviceProtocol::loadRemoteID loadRemoteID) {
  _settings.setSettings(settings->getSettings());

  _radio->begin();

  //If there was a previous connection, try to re-establish it.
  if(checkConnected()) {
    apply_settings(&_settings);

    //Load the Remote ID.
    uint8_t id[5];
    loadRemoteID(id);

    _radio->openWritingPipe(id);
    _radio->openReadingPipe(1, _deviceId);

    _radio->startListening();

    /*If the remote is still connected, it should be sending regular data
    _settings.getCommsFrequency() per second, so we will wait for 3 time
    periods before giving up.*/
    if(wait_till_available(round(3000.0 / _settings.getCommsFrequency())) == -1) {
      return -1;
    }

    for(uint8_t i = 0; i < 5; i++) {
      _remoteId[i] = id[i];
    }
    _isConnected = true;


    return 1;

  }

  return 0;
}

int8_t DeviceProtocol::pair(DeviceProtocol::saveRemoteID saveRemoteID) {
  if(isConnected()) {
    return RC_ERROR_ALREADY_CONNECTED;
  }

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

int8_t DeviceProtocol::connect(DeviceProtocol::loadRemoteID loadRemoteID,
                               DeviceProtocol::setConnected setConnected) {
  if(isConnected()) {
    return RC_ERROR_ALREADY_CONNECTED;
  }

  uint8_t connectSuccess = 0;
  uint8_t test = 0;

  uint8_t remoteId[5];
  loadRemoteID(remoteId);

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
  setConnected(true);

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

int8_t DeviceProtocol::update(uint16_t channels[], uint8_t telemetry[],
                              DeviceProtocol::setConnected setConnected) {
  if(!isConnected()) {
    return RC_ERROR_NOT_CONNECTED;
  }

  uint8_t packet[_settings.getPayloadSize()];

  int8_t packetStatus = 0;
  int8_t status = 0;

  //Load a transmission, and send an ack payload.
  packetStatus = check_packet(packet,
                              _settings.getPayloadSize() * sizeof(uint8_t),
                              telemetry, _settings.getPayloadSize());


  //read through each transmission we have gotten since the last update
  while(packetStatus == 1) {

    //Covert packet to channels

    //Check if the packet is a channel packet
    if((packet[0] & 0xF0) == _PACKET_CHANNELS) {

      status = 1;

      for(uint8_t i = 0; i < _settings.getNumChannels(); i++) {
        channels[i] = ((packet[i * 2 + 1] << 8)) | (packet[i * 2 + 2]);
      }

      //If the packet is a Disconnect Packet
    } else if(packet[0] == _PACKET_DISCONNECT) {
      if(!_settings.getEnableAck()) {
        _radio->stopListening();
        delay(50);
        _radio->write(const_cast<uint8_t*>(&_ACK), 1);
        _radio->startListening();
      }

      _isConnected = false;
      setConnected(false);

      //If the packet is a Reconnect Packet
    } else if(packet[0] == _PACKET_RECONNECT) {
      if(!_settings.getEnableAck()) {
        _radio->stopListening();
        delay(20);
        _radio->write(const_cast<uint8_t*>(&_ACK), 1);
        _radio->startListening();
      }
    }

    //Load a transmission.
    packetStatus = check_packet(packet,
                                _settings.getPayloadSize() * sizeof(uint8_t));
  }

  if(packetStatus < 0) {
    status = packetStatus;
  }

  return status;
}

RCSettings* DeviceProtocol::getSettings() {
  return &_settings;
}