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

void RCTelemetry::setRPM(int16_t rpm) {
  if(isRPMEnabled()) {
    *_rpmLoc = (rpm >> 8) & 255;
    *(_rpmLoc + 1) = rpm & 255;
  }
}

int16_t RCTelemetry::getRPM() const {
  if(isRPMEnabled()) {
    return (static_cast<int16_t>(*_rpmLoc) << 8) | (*(_rpmLoc + 1));
  } else {
    return 0;
  }
}

bool RCTelemetry::isRPMEnabled() const {
  return _rpmLoc;
}

void RCTelemetry::setGPS(float x, float y, float z) {
  if(isGPSEnabled()) {
    write_float(x, _gpsLoc);
    write_float(y, _gpsLoc + FLOAT_SIZE);
    write_float(z, _gpsLoc + (FLOAT_SIZE * 2));
  }
}

void RCTelemetry::getGPS(float &x, float &y, float &z) const {
  if(isGPSEnabled()) {
    x = get_float(_gpsLoc);
    y = get_float(_gpsLoc + FLOAT_SIZE);
    z = get_float(_gpsLoc + (FLOAT_SIZE * 2));
  } else {
    x = 0;
    y = 0;
    z = 0;
  }
}

bool RCTelemetry::isGPSEnabled() const {
  return _gpsLoc;
}

void RCTelemetry::setAlarm(uint8_t alarmNumber, bool value) {
  if(numAlarmsEnabled() > alarmNumber) {
    if(value) {
      *_alarmLoc = (*_alarmLoc) | (1 << (alarmNumber + _alarmBin));
    } else {
      *_alarmLoc = (*_alarmLoc) & ~(1 << (alarmNumber + _alarmBin));
    }
  }
}

bool RCTelemetry::getAlarm(uint8_t alarmNumber) const {
  if(numAlarmsEnabled() > alarmNumber) {
    return (*_alarmLoc) & (1 << (alarmNumber + _alarmBin)) ;
  } else {
    return false;
  }
}

uint8_t RCTelemetry::numAlarmsEnabled() const {
  return _alarmLoc ? _alarmNum : 0;
}

void RCTelemetry::load_pointers() {
  uint8_t bits = _settings->getTelemetryChannels();
  uint8_t i = 0;
  uint8_t binChannel = 0;
  uint8_t binI = 8;

  //Battery channel
  if(bits & RC_TELEM_BATTERY) {
    _batteryLoc = _telemetry + i;
    i += BATTERY_SIZE;
  } else {
    _batteryLoc = NULL;
  }

  //Current Channel
  if(bits & RC_TELEM_CURRENT) {
    _currentLoc = _telemetry + i;
    i += CURRENT_SIZE;
  } else {
    _currentLoc = NULL;
  }

  //Temperature Channel
  if(bits & RC_TELEM_TEMPERATURE) {
    _tempLoc = _telemetry + i;
    i += TEMPERATURE_SIZE;
  } else {
    _tempLoc = NULL;
  }

  //RPM Channel
  if(bits & RC_TELEM_RPM) {
    _rpmLoc = _telemetry + i;
    i += RPM_SIZE;
  } else {
    _rpmLoc = NULL;
  }

  //GPS Channel
  if(bits & RC_TELEM_GPS) {
    _gpsLoc = _telemetry + i;
    i += GPS_SIZE;
  } else {
    _gpsLoc = NULL;
  }

  //Alarm Channels
  //If any of the alarms ar enabled.
  if(bits & RC_TELEM_ALARM3) {
    //Determin the number of alarms
    if((bits & RC_TELEM_ALARM3) == RC_TELEM_ALARM3) {
      _alarmNum = 3;
    } else if(bits & RC_TELEM_ALARM1) {
      _alarmNum = 1;
    } else {
      _alarmNum = 2;
    }

    //If the current binary channel is full, make a new one.
    if(binI >= 8 - (ALARM_BIT_SIZE * _alarmNum)) {
      binI = 0;
      binChannel = i;
      i++;
    }

    //Set the location for the alarm byte and its bits.
    _alarmLoc = _telemetry + binChannel;
    _alarmBin = binI;
    binI += ALARM_BIT_SIZE * _alarmNum;
  } else {
    _alarmLoc = NULL;
    _alarmNum = 0;
  }
}

uint8_t RCTelemetry::calculate_size(const RCSettings* settings) {
  uint8_t i = 0;
  uint8_t binI = 8;

  uint8_t bits = settings->getTelemetryChannels();

  //Battery
  if(bits & RC_TELEM_BATTERY) {
    i += BATTERY_SIZE;
  }
  //Current
  if(bits & RC_TELEM_CURRENT) {
    i += CURRENT_SIZE;
  }
  //Temperature
  if(bits & RC_TELEM_TEMPERATURE) {
    i += TEMPERATURE_SIZE;
  }
  //RPM
  if(bits & RC_TELEM_RPM) {
    i += RPM_SIZE;
  }
  //GPS
  if(bits & RC_TELEM_GPS) {
    i += GPS_SIZE;
  }
  //Alarms
  if(bits & RC_TELEM_ALARM3) {
    uint8_t alarmsize;
    //Determin the number of alarms
    if((bits & RC_TELEM_ALARM3) == RC_TELEM_ALARM3) {
      alarmsize = 3;
    } else if(bits & RC_TELEM_ALARM1) {
      alarmsize = 1;
    } else {
      alarmsize = 2;
    }

    //If the current binary channel is full, make a new one.
    if(binI >= 8 - (ALARM_BIT_SIZE * alarmsize)) {
      binI = 0;
      i++;
    }

    binI += ALARM_BIT_SIZE * alarmsize;
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