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

  /**
   * Set the RPM
   *
   * @param rpm
   */
  void setRPM(int16_t rpm);
  /**
   * Get the rpm from setRPM()
   *
   * @return 0 if disabled
   * @return rpm
   */
  int16_t getRPM() const;
  /**
   * Check if the RPM channel is enabled
   *
   * @return true if enabled
   */
  bool isRPMEnabled() const;

  /**
   * Set the location
   *
   * @note I don't know much about gps, so the parameters here may not be right
   *
   * @param x longitude
   * @param y latitude
   * @param z elevation
   */
  void setGPS(float x, float y, float z);
  /**
   * Get the location from setGPS()
   *
   * The x,y,z are set when getGPS() is called
   *
   * @note If GPS is disabled, x,y,z are set to 0
   *
   * @param x longitude
   * @param y latitude
   * @param z elevation
   */
  void getGPS(float &x, float &y, float &z) const;
  /**
   * Check if GPS channel is enabled
   *
   * @return true if enabled
   */
  bool isGPSEnabled() const;

  /**
   * Set an alarm
   *
   * No alarm is set if the alarmNumber is greater than the numAlarmsEnabled().
   *
   * @param alarmNumber alarms start at 0
   * @param value
   */
  void setAlarm(uint8_t alarmNumber, bool value);
  /**
   * Get the value of an alarm from setAlarm()
   *
   * If the alarmNumber is greater than numAlarmsEnabled()
   *
   * @param alarmNumber alarms start at 0
   *
   * @return false if alarmNumber is disabled or inactive
   * @return true if alarmNumber is active
   */
  bool getAlarm(uint8_t alarmNumber) const;
  /**
   * Get the number of alarms enabled
   *
   * There are at most 3 alarms enabled
   *
   * @return number of alarms enabled
   */
  uint8_t numAlarmsEnabled() const;

private:
  static const uint8_t FLOAT_SIZE = 3;
  static const uint8_t SMALL_FLOAT_SIZE = 2;

  static const uint8_t BATTERY_SIZE = SMALL_FLOAT_SIZE;
  static const uint8_t CURRENT_SIZE = FLOAT_SIZE;
  static const uint8_t TEMPERATURE_SIZE = 1;
  static const uint8_t RPM_SIZE = 2;
  static const uint8_t GPS_SIZE = FLOAT_SIZE * 3;
  static const uint8_t ALARM_BIT_SIZE = 1;

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
  uint8_t* _rpmLoc;
  uint8_t* _gpsLoc;
  uint8_t* _alarmLoc;
  uint8_t _alarmBin;//location of alarm in alarmLoc bits
  uint8_t _alarmNum;//number of alarms

};

#endif