#include "rcTelemetry.h"

RCTelemetry::RCTelemetry(const RCSettings* settings) {
  _settings = settings;

  _size = calculate_size(_settings);
  _telemetry = new uint8_t[_size];

  load_pointers();
}

RCTelemetry::~RCTelemetry() {
  delete[] _telemetry;
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
  if(isBatteryEnabled()) {
    write_small_float(battery, _batteryLoc);
  }
}

float RCTelemetry::getBattery() const {
  if(isBatteryEnabled()) {
    return get_small_float(_batteryLoc);
  }
  return 0;
}

bool RCTelemetry::isBatteryEnabled() const {
  return _batteryLoc;
}

void RCTelemetry::setCurrent(float current) {
  if(isCurrentEnabled()) {
    write_float(current, _currentLoc);
  }
}

float RCTelemetry::getCurrent() const {
  if(isCurrentEnabled()) {
    return get_float(_currentLoc);
  }
  return 0;
}

bool RCTelemetry::isCurrentEnabled() const {
  return _currentLoc;
}

void RCTelemetry::setTemperature(int8_t temperature) {
  if(isTemperatureEnabled()) {
    *_tempLoc = temperature;
  }
}

int8_t RCTelemetry::getTemperature() const {
  if(isTemperatureEnabled()) {
    return *_tempLoc;
  }
  return 0;
}

bool RCTelemetry::isTemperatureEnabled() const {
  return _tempLoc;
}

void RCTelemetry::load_pointers() {
  uint8_t bits = _settings->getTelemetryChannels();
  uint8_t i = 0;

  if(bits & RC_TELEM_BATTERY) {
    _batteryLoc = _telemetry + i;
    i += BATTERY_SIZE;
  } else {
    _batteryLoc = NULL;
  }

  if(bits & RC_TELEM_CURRENT) {
    _currentLoc = _telemetry + i;
    i += CURRENT_SIZE;
  } else {
    _currentLoc = NULL;
  }

  if(bits & RC_TELEM_TEMPERATURE) {
    _tempLoc = _telemetry + i;
    i += TEMPERATURE_SIZE;
  } else {
    _tempLoc = NULL;
  }
}

uint8_t RCTelemetry::calculate_size(const RCSettings* settings) {
  uint8_t i = 0;

  uint8_t bits = settings->getTelemetryChannels();

  if(bits & RC_TELEM_BATTERY) {
    i += BATTERY_SIZE;
  }
  if(bits & RC_TELEM_CURRENT) {
    i += CURRENT_SIZE;
  }
  if(bits & RC_TELEM_TEMPERATURE) {
    i += TEMPERATURE_SIZE;
  }

  return i;
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