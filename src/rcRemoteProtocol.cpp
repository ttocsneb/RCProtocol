#include <RF24.h>
#include <printf.h>

#include "rcRemoteProtocol.h"
#include "rcSettings.h"

RemoteProtocol::RemoteProtocol(RF24* tranceiver, const uint8_t remoteId[]) {
  //initialize all primitive variables
  _isConnected = false;

  for(uint8_t i = 0; i < 5; i++) {
    _deviceId[i] = 0;
  }

  _timer = millis();

  _radio = tranceiver;
  _remoteId = remoteId;
}

void RemoteProtocol::begin() {
  _radio->begin();

  _radio->stopListening();

}

int8_t RemoteProtocol::pair(RemoteProtocol::saveSettings saveSettings) {
  uint8_t settings[32];
  uint8_t deviceId[5];

  _radio->setPALevel(RF24_PA_LOW);

  apply_settings(&_pairSettings);

  _radio->stopListening();
  _radio->openWritingPipe(_PAIR_ADDRESS);

  //clear the buffer of any unread messages.
  flush_buffer();

  //Send the remote's id until a receiver has acknowledged
  if(force_send(const_cast<uint8_t*>(_remoteId), 5, RC_TIMEOUT) != 0) {
    return RC_ERROR_TIMEOUT;
  }

  //Start listening for the device to send data back
  _radio->openReadingPipe(1, _remoteId);
  _radio->startListening();

  //wait until data is available, if it takes too long, error lost connection
  if(wait_till_available(RC_CONNECT_TIMEOUT) != 0) {
    return RC_ERROR_LOST_CONNECTION;
  }

  //read the deviceId
  _radio->read(&deviceId, 5);


  //wait until data is available, if it takes too long, error lost connection
  if(wait_till_available(RC_CONNECT_TIMEOUT) != 0) {
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

  apply_settings(&_pairSettings);

  //We don't yet open a writing pipe as we don't know who we will write to.
  _radio->openReadingPipe(1, _remoteId);

  _radio->startListening();

  //Wait for communications, timeout error if it takes too long
  if(wait_till_available(RC_TIMEOUT) != 0) {
    return RC_ERROR_TIMEOUT;
  }

  //Read the device ID
  _radio->read(&_deviceId, 5);

  //Check if we can pair with the device
  valid = checkIfValid(_deviceId, settings);
  _settings.setSettings(settings);

  //Start Writing
  _radio->stopListening();

  //We now know who we will be writing to, so open the writing pipe
  _radio->openWritingPipe(_deviceId);

  //Delay so that the device has time to switch modes
  delay(200);

  //If the device is allowed to connect, send the _ACK command, else _NACK
  if(valid) {
    if(_radio->write(const_cast<uint8_t*>(&_ACK), 1) == false) {
      return RC_ERROR_LOST_CONNECTION;
    }
  } else {
    if(_radio->write(const_cast<uint8_t*>(&_NACK), 1) == false) {
      return RC_ERROR_LOST_CONNECTION;
    }
    return RC_ERROR_CONNECTION_REFUSED;
  }

  //Set the radio settings to the settings specified by the receiver.
  apply_settings(&_settings);

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

    if(wait_till_available(RC_CONNECT_TIMEOUT) != 0) {
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

int8_t RemoteProtocol::send_packet(void* data, uint8_t dataSize,
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

  if(!isConnected()) {
    return RC_ERROR_NOT_CONNECTED;
  }

  uint8_t packet[_settings.getPayloadSize()];

  uint8_t i = 0;

  //Set the Packet type
  packet[i++] = _PACKET_CHANNELS + 0;
  //Set the payload data
  for(; i < min(_settings.getPayloadSize(), _settings.getNumChannels() * 2 + 1);
      i += 2) {
    packet[i] = (channels[(i - 1) / 2] >> 8) & 0x00FF;
    packet[i + 1] = channels[(i - 1) / 2] & 0x00FF;
  }
  //Set the rest of the packet to 0.
  for(; i < _settings.getPayloadSize(); i++) {
    packet[i] = 0;
  }


  //Send the packet.
  int8_t status = send_packet(packet,
                              sizeof(uint8_t) * _settings.getPayloadSize(),
                              telemetry, sizeof(uint8_t) * _settings.getPayloadSize());


  //If the tick was too long, and there are no errors, set the return to Tick To Short
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

RCSettings* RemoteProtocol::getSettings() {
  return &_settings;
}