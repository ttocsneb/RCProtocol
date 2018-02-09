#ifndef __RCTELEMETRY_H__
#define __RCTELEMETRY_H__

#include <RF24.h>
#include "rcSettings.h"

class RCTelemetry {
public:

  RCTelemetry(const RCSettings* settings);

  RCTelemetry(const RCSettings* settings, uint8_t* telemetry);

  uint8_t getTelemetrySize() {
    return _size;
  }

  void loadTelemetry(uint8_t* telemetry);
  uint8_t* getTelemetry() const;

  void setBattery(float battery);
  float getBattery() const;

  void setCurrent(float current);
  float getCurrent() const;

  void setTemperature(int8_t temperature);
  int8_t getTemperature() const;



private:

  /**
   * Write a float to a char array.
   *
   * there needs to be 3 bytes available to write to
   */
  static void write_float(float number, uint8_t* location);
  static float get_float(uint8_t* location);

  static void write_small_float(float number, uint8_t* location);
  static float get_small_float(uint8_t* location);

  static uint8_t calculate_size(const RCSettings* settings);

  void load_pointers();

  const RCSettings* _settings;

  uint8_t* _telemetry;
  uint8_t _size;

  //The pointers to each item, which is located somewhere in _telemetry
  uint8_t* _batteryLoc;
  uint8_t* _currentLoc;
  uint8_t* _tempLoc;

};

#endif