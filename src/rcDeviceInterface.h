/**
 * reDeviceInterface.h
 *
 * Holds #DeviceInterface used by #DeviceProtocol
 *
 * Created by Benjamin Jacobs, February 3, 2018
 */

#ifndef __RCDEVICEINTERFACE_H__
#define __RCDEVICEINTERFACE_H__

#include <RF24.h>

/**
 * DeviceInterface is an interface that is implemented outside of the library.
 *
 * This holds user-defined functions that need to be implemented in order to work properly.
 *
 * DeviceInterface is used by DeviceProtocol
 *
 * @see #DeviceProtocol
 */
class DeviceInterface {
public:

  /**
   * Save the transmitter id to non-volitile memory.
   *
   * the id needs to be saved somewhere non-volitile, ie. EEPROM, sd card,
   * etc.
   *
   * @param id char[5] array
   */
  virtual void saveRemoteID(const uint8_t* id) = 0;

  /**
   * Load the transmitter id from non-volitile memory.
   *
   * Load the id from saveRemoteID() into the given char array.
   *
   * @param id empty char[5] array
   */
  virtual void loadRemoteID(uint8_t* id) = 0;

  /**
   * Check if the device was previously connected to the remote.
   *
   * Load the value from setConnected()
   *
   * @return value of setConnected()
   */
  virtual bool checkConnected() = 0;

  /**
   * Save the status of the device's connection to non-volitile memory
   *
   * Save to non-volitile memory: EEPROM, SD card, etc. the current connection status
   *
   * @param connected the value to save.
   */
  virtual void setConnected(bool connected) = 0;
};

#endif