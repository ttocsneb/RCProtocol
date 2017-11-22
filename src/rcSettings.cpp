#include <RF24.h>

#include "rcSettings.h"

RCSettings::RCSettings() {
    setEnableDynamicPayload(false);
    setEnableAck(true);
    setEnableAckPayload(true);
    setStartChannel(0);
    setDataRate(RF24_1MBPS);
    setPayloadSize(32);
    setCommsFrequency(60);
}

void RCSettings::setSettings(const uint8_t* settings) {
    for(int i=0; i<32; i++) {
        _settings[i] = settings[i];
    }
}

uint8_t* RCSettings::getSettings() {
    return _settings;
}

void RCSettings::setEnableDynamicPayload(bool enable) {
    //put Enable Dynamic Payload in bit 0 of byte 0
    //0b00000001
    _settings[0] = (enable ? (_settings[0] | 1) : (_settings[0] & (~1)));
}

bool RCSettings::getEnableDynamicPayload() {
    return _settings[0] & 1;
}

void RCSettings::setEnableAck(bool enable) {
    //Put Enable Ack in bit 1 of byte 0
    //0b00000010: 2
    _settings[0] = (enable ? (_settings[0] | 2) : (_settings[0] & (~2)));
}

bool RCSettings::getEnableAck() {
    return (_settings[0] >> 1) & 1;
}

void RCSettings::setEnableAckPayload(bool enable) {
    //Put Enable Ack Payload in bit 2 of byte 0
    //0b00000100: 4
    _settings[0] = (enable ? (_settings[0] | 4) : (_settings[0] & (~4)));
}

bool RCSettings::getEnableAckPayload() {
    return (_settings[0] >> 2) & 1;
}

void RCSettings::setDataRate(rf24_datarate_e datarate) {
    //Put Datarate in bits 3 and 4 of byte 0
    //0b00011000: 24
    _settings[0] = (_settings[0] & (~24)) | (datarate << 3);
}

rf24_datarate_e RCSettings::getDataRate() {
    uint8_t datarate = (_settings[0] >> 3) & 3;
    switch(datarate) {
    case RF24_2MBPS:
        return RF24_2MBPS;
    case RF24_250KBPS:
        return RF24_250KBPS;
    case RF24_1MBPS:
    default:
        return RF24_1MBPS;
    }
}

void RCSettings::setStartChannel(uint8_t channel) {
    _settings[1] = channel;
}

uint8_t RCSettings::getStartChannel() {
    return _settings[1];
}

void RCSettings::setPayloadSize(uint8_t payload)  {
    _settings[2] = payload;
}

uint8_t RCSettings::getPayloadSize() {
    return _settings[2];
}

void RCSettings::setCommsFrequency(uint8_t frequency) {
    _settings[3] = frequency;
}

uint8_t RCSettings::getCommsFrequency() {
    return _settings[3];
}