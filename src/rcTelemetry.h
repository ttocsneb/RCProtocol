#ifndef __RCTELEMETRY_H__
#define __RCTELEMETRY_H__

#include <RF24.h>
#include "rcSettings.h"

class RCTelemetry {
public:

  /**
   * Create a new empty RCTelemetry object
   *
   * An RCSettings object is required to allocate the proper amount of memory
   * for the telemetry.  It uses RCSettings.getTelemetryChannels()
   *
   * @param settings
   */
  RCTelemetry(const RCSettings* settings);

  /**
   * Destroy the RCTelemetry object.
   *
   * This deletes the allocated memory for
   * RCTelemetry.
   */
  ~RCTelemetry();

  /**
   * Get the size of the Telemetry array
   *
   * @return  telemetrySize
   */
  uint8_t getTelemetrySize() {
    return _size;
  }

  /**
   * Load telemetry array
   *
   * @param telemetry
   */
  void loadTelemetry(uint8_t* telemetry);
  /**
   * Get the Telemetry array
   *
   * @note the size of the array is found from getTelemetrySize()
   *
   * @return telemetry
   */
  uint8_t* getTelemetry() const;

  /**
   * Set the battery voltage.
   *
   * The voltage should be in terms of cell voltage ie. 4.20
   *
   * @param battery cell voltage
   */
  void setBattery(float battery);
  /**
   * Get the voltage from setBatter()
   *
   * @note if battery channel is disabled, this will return 0
   *
   * @return 0 if battery is disabled
   * @return cell voltage
   */
  float getBattery() const;
  /**
   * Check if Battery is enabled.
   *
   * @return true if Battery channel is enabled.
   */
  bool isBatteryEnabled() const;

  /**
   * Set the battery current.
   *
   * The current should be in amps
   *
   * @param current
   */
  void setCurrent(float current);
  /**
   * Get the current from setCurrent()
   *
   * @note if current channel is disabled, this will return 0
   *
   * @return 0 if disabled
   * @return current
   */
  float getCurrent() const;
  /**
   * Check if the Current Channel is enabled
   *
   * @return true if Current channel is enabled
   */
  bool isCurrentEnabled() const;

  /**
   * Set the temperature
   *
   * The temperature should be in Celsius, and has a range of -128 - 127
   *
   * @param temperature
   *
   */
  void setTemperature(int8_t temperature);
  /**
   * Get the temperature from setTemperature()
   *
   * @note if Temperature is disabled, this will return 0
   *
   * @return 0 if disabled
   * @return temperature
   */
  int8_t getTemperature() const;
  /**
   * Check if the Temperature channel is enabled
   *
   * @return true if Temperature channel is enabled
   */
  bool isTemperatureEnabled() const;

private:

  static const uint8_t BATTERY_SIZE = 2;
  static const uint8_t CURRENT_SIZE = 3;
  static const uint8_t TEMPERATURE_SIZE = 1;

  /**
   * Write a float to a char array.
   *
   * there needs to be 3 bytes available to write to
   */
  static void write_float(float number, uint8_t* location);
  static float get_float(uint8_t* location);

  /**
   * Write a float to a char array.
   *
   * there needs to be 2 bytes available to write to
   */
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