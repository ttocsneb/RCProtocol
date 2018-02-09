/**
 * rcRemoteInterface.h
 *
 * Holds #RemoteInterface used by #RemoteProtocol
 *
 * Created by Benjamin Jacobs, February 3, 2018
 */

#ifndef __RCREMOTEINTERFACE_H__
#define __RCREMOTEINTERFACE_H__

#include <RF24.h>

/**
 * RemoteInterface is an interface that is implemented outside of the library.
 *
 * This holds user-defined functions that need to be implemented in order
 * to work properly
 *
 * RemoteInterface is used by RemoteProtocol
 */
class RemoteInterface {
public:

  /**
   * Save settings to non-volitile memory.
   *
   * Should save both the id, and settings to non-volitile memory such as
   * EEPROM, SD card, etc.
   *
   * @param id char[5] array
   * @param settings char[32] array
   */
  virtual void saveSettings(const uint8_t* id, const uint8_t* settings) = 0;

  /**
   * Check if the given id is a valid id, then load the settings associated
   * with that id.
   *
   * If the id is not found, then the settings should not be changed, and
   * return false.
   *
   * Example code:
   * @code
   * if(findID(id) == true) {
   *   loadSettings(settings);
   *   return true;
   * } else {
   *   return false;
   * }
   * @endcode
   *
   * @param id char[5] array
   * @param settings empty char[32] array
   *
   * @return true if the check was successful
   */
  virtual bool checkIfValid(const uint8_t* id, uint8_t* settings) = 0;

  /**
   * Load the id of the last connected device.
   *
   * This should load the id set by setLastConnection() into id
   *
   * @note This is used in RemoteProtocol::begin(), so any objects used should
   * already be initialized before RemoteProtocol::begin() is called.
   *
   * @param id empty char[5] array
   */
  virtual void getLastConnection(uint8_t* id) = 0;

  /**
   * Save the id of the current device to non-volitile memory.
   *
   * The id should be saved to non-volitile memory such as EEPROM, SD card,
   * etc.
   *
   * @param id char[5] array
   */
  virtual void setLastConnection(const uint8_t* id) = 0;
};

#endif