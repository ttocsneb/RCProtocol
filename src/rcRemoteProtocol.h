/*
   rcRemoteProtocol.h - Library for Remote for RCProtocol.
   Created by Benjamin Jacobs, October 1, 2017
*/

#ifndef __RCREMOTEPROTOCOL_H__
#define __RCREMOTEPROTOCOL_H__

#include <Arduino.h>
#include <RF24.h>

#include "rcSettings.h"

#ifndef __RF24_H__
#error "rcRemoteProtocol Requires the tmrh20 RF24 Library: https://github.com/nRF24/RF24"
#endif


//Userdefined Constants

#ifndef RC_TIMEOUT
#define RC_TIMEOUT 15000
#endif

#ifndef RC_CONNECT_TIMEOUT
#define RC_CONNECT_TIMEOUT 2500
#endif

//Error constants

/**
 * Communications have been established, but since lost it
 */
#define RC_ERROR_LOST_CONNECTION -1
/**
 * No connection has been made
 */
#define RC_ERROR_TIMEOUT -2
/**
 * Data that was received does not match expectations
 */
#define RC_ERROR_BAD_DATA -3
/**
 * Receiver was refused to connect
 */
#define RC_ERROR_CONNECTION_REFUSED -4

/**
 * Communication Protocol for transmitters
 */
class RemoteProtocol {
public:
  /**
   * Save settings to non-volitile memory, such as EEPROM
   * 
   * The function should save both the id, and settings to some form of non-volitile memory.
   * This can be done in any way as long as the data can be retrieved, and checked.
   * 
   * @param id 5 byte char array containing the ID of the receiver
   * @param settings 32 byte array of settings
   */
  typedef void (saveSettings)(const uint8_t* id, const uint8_t* settings);
  /**
   * Check if the given id has been paired, and load the corresponding settings into the
   * settings array.
   * 
   * If the id is not found, the settings should not be changed, and return false.
   * 
   * Here is a heavily simplified example:
   * @code
   * if(findID(id) == true) {
   *   loadSettings(settings);
   *   return true;
   * } else {
   *   return false;
   * }
   * @endcode
   * 
   * @param id 5 byte char array containing the ID of the receiver
   * @param settings 32 byte array to be loaded with the settings of the ID
   * 
   * @return true if the check was successful
   */
  typedef bool (checkIfValid)(const uint8_t* id, uint8_t* settings);

  /**
   * Constructor
   * 
   * Creates a new instance of the protocol. You create an instance and send a reference
   * to the RF24 driver as well as the id of the remote
   * 
   * @param tranceiver A reference to the RF24 chip, this allows you to create your own instance,
   * allowing multi-platform support
   * @param remoteId The 5 byte char array of the remotes ID: ex "MyRmt"
   */
  RemoteProtocol(RF24 *tranceiver, const uint8_t remoteId[]);

  /**
   * Begin the Protocol
   * 
   * @note There is no need to begin the RF24 driver, as this function does this for you
   */
  void begin();

  /**
   * Attempt to pair with a receiver
   * 
   * @note The receiver you are trying to pair with should also be in pair mode
   * 
   * @param saveSettings A function pointer to save the settings of the paired device.
   * 
   * @return 0 if successful
   * @return #RC_ERROR_TIMEOUT if no receiver was found.
   * @return #RC_ERROR_LOST_CONNECTION if receiver stoped replying
   */
  int8_t pair(saveSettings saveSettings);

  /**
   * Attempt to connect with a previously paired device
   * 
   * @note The receiver should have already been paired with the remote, and in connect mode
   * 
   * @param checkIfValid A function pointer to check if the found device has been paired, and to
   * load the settings
   * 
   * @return 0 if successful
   * @return #RC_ERROR_TIMEOUT if no receiver was found.
   * @return #RC_ERROR_LOST_CONNECTION if receiver stopped replying
   * @return #RC_ERROR_CONNECTION_REFUSED if the receiver is not on the pair list.
   * @return #RC_ERROR_BAD_DATA if the settings are not set properly on both devices
   */
  int8_t connect(checkIfValid checkIfValid);

  /**
   * Update the communications with the currently connected device
   * 
   * @note We should have already be connected with a device before calling update, see connect()
   */
  int8_t update();


private:
  const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
  const uint8_t _YES = 0x6; //ACKNOWLEDGE
  const uint8_t _NO = 0x15; //NEGATIVE ACKNOWLEDGE
  const uint8_t _TEST = 0x2; //START OF TEXT


  const uint8_t *_remoteId;
  uint8_t _deviceId[5];

  RCSettings _settings;
  RCSettings _pairSettings;

  RF24 *_radio;

  int8_t _forceSend(void *buf, uint8_t size, uint32_t timeout);
  int8_t _waitTillAvailable(uint32_t timeout);
  void _flushBuffer();

  void _applySettings(RCSettings *settings);
};

#endif
