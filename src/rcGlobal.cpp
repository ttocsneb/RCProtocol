#include "rcGlobal.h"

RCGlobal::RCGlobal() {

  //Setup Pair Settings
  _pairSettings.setEnableDynamicPayload(false);
  _pairSettings.setEnableAck(true);
  _pairSettings.setEnableAckPayload(false);
  _pairSettings.setDataRate(RF24_1MBPS);
  _pairSettings.setStartChannel(63);
  _pairSettings.setPayloadSize(32);
  _pairSettings.setRetryDelay(7);
}

int8_t RCGlobal::force_send(void* buf, uint8_t size, unsigned long timeout) {
  uint32_t t = millis();
  bool ack = false;
  while(!ack && millis() - t < timeout) {
    ack = _radio->write(buf, size);
  }
  if(!ack) {
    return -1;
  }

  return 0;
}

int8_t RCGlobal::wait_till_available(unsigned long timeout) {
  uint32_t t = millis();
  while(!_radio->available() && millis() - t < timeout) {
    delay(16);
  }
  if(millis() - t >= timeout) {
    return -1;
  }

  return 0;
}

void RCGlobal::apply_settings(RCSettings* settings) {
  //Enable/disable Dynamic Payloads, and set payload size
  if(settings->getEnableDynamicPayload()) {
    _radio->enableDynamicPayloads();
  } else {
    _radio->disableDynamicPayloads();
    _radio->setPayloadSize(settings->getPayloadSize());
  }

  //Enable/Disable autoack, and custom payloads.
  _radio->setAutoAck(settings->getEnableAck());
  if(settings->getEnableAck() && settings->getEnableAckPayload()) {
    _radio->enableAckPayload();
  }

  //Set the channel
  _radio->setChannel(settings->getStartChannel());

  //Set the data rate
  _radio->setDataRate(settings->getDataRate());

  //Set the Retry delay.  I might add retry number as an option later.
  _radio->setRetries(settings->getRetryDelay(), 15);
}

void RCGlobal::flush_buffer() {
  uint8_t tmp;
  while(_radio->available()) {
    _radio->read(&tmp, 1);
  }
}