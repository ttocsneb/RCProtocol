#include "rcTelemetry.h"

RCTelemetry::RCTelemetry(const RCSettings* settings) {
  _settings = settings;

  //TODO make _telemetry dynamically allocated based on settings.
  _size = calculate_size(_settings);
  _telemetry = new uint8_t[_size];

  load_pointers();
}

RCTelemetry::RCTelemetry(const RCSettings* settings,
                         uint8_t* telemetry) : RCTelemetry(settings) {

  loadTelemetry(telemetry);
}

void RCTelemetry::loadTelemetry(uint8_t* telemetry) {
  for(uint8_t i = 0; i < _size; i++) {
    _telemetry[i] = telemetry[i];
  }
}

uint8_t* RCTelemetry::getTelemetry() const {
  return _telemetry;
}

void RCTelemetry::setBattery(float battery) {
  if(_batteryLoc) {
    write_small_float(battery, _batteryLoc);
  }
}

float RCTelemetry::getBattery() const {
  if(_batteryLoc) {
    return get_small_float(_batteryLoc);
  }
  return 0;
}

void RCTelemetry::setCurrent(float current) {
  if(_currentLoc) {
    write_float(current, _currentLoc);
  }
}

float RCTelemetry::getCurrent() const {
  if(_currentLoc) {
    return get_float(_currentLoc);
  }
  return 0;
}

void RCTelemetry::setTemperature(int8_t temperature) {
  if(_tempLoc) {
    *_tempLoc = temperature;
  }
}

int8_t RCTelemetry::getTemperature() const {
  if(_tempLoc) {
    return *_tempLoc;
  }
  return 0;
}

void RCTelemetry::load_pointers() {
  //TODO load telemetry pointers
  _batteryLoc = NULL;
  _currentLoc = NULL;
  _tempLoc = NULL;
}

uint8_t RCTelemetry::calculate_size(const RCSettings* settings) {
  //TODO actually calculate size;
  return 32;
}

void RCTelemetry::write_float(float number, uint8_t* location) {
  uint8_t* num1 = location;
  uint8_t* num2 = 1 + location;
  uint8_t* exp = 2 + location;

  int16_t n = static_cast<uint16_t>(number);
  uint8_t e = 0;
  //find the proper power
  while(abs(n) <= 65535 && n != number * pow(2, e) && e < 255) {
    n = static_cast<uint16_t>(number * pow(2, ++e));
  }

  if(abs(n) > 65535) {
    n = static_cast<uint16_t>(number * pow(2, --e));
  }

  *num1 = (n >> 8) & 255;
  *num2 = n & 255;
  *exp = e;
}

float RCTelemetry::get_float(uint8_t* location) {
  const uint8_t* NUM1 = location;
  const uint8_t* NUM2 = 1 + location;
  const uint8_t* EXP = 2 + location;

  int16_t num = (static_cast<uint16_t>(*NUM1) << 8) | *NUM2;

  uint8_t e = *EXP;

  float number = static_cast<float>(num) / pow(2, e);

  return number;
}

void RCTelemetry::write_small_float(float number, uint8_t* location) {
  uint8_t* num1 = location;
  uint8_t* num2 = 1 + location;

  int16_t n = static_cast<uint16_t>(number);
  uint8_t e = 0;
  //find the proper power
  while(abs(n) <= 2047 && n != number * pow(2, e) && e < 30) {
    e += 2;
    n = static_cast<uint16_t>(number * pow(2, e));
  }

  if(abs(n) > 2047) {
    e -= 2;
    n = static_cast<uint16_t>(number * pow(2, e));
  }

  *num1 = ((n >> 8) & 15) | ((e / 2) << 4);
  *num2 = n & 255;
}

float RCTelemetry::get_small_float(uint8_t* location) {
  const uint8_t* NUM1 = location;
  const uint8_t* NUM2 = 1 + location;

  uint16_t n = (static_cast<uint16_t>(*NUM1) << 8) | *NUM2;

  int16_t num = n & 4095;

  //make the number negative
  if(num & 2048) {
    num = num | -4096;
  }

  uint8_t e = ((n >> 12) & 15) * 2;

  float number = static_cast<float>(num) / pow(2, e);

  return number;
}