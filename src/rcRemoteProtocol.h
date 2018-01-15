/*
   rcRemoteProtocol.h - Library for Remote for RCProtocol.
   Created by Benjamin Jacobs, October 1, 2017
*/

#ifndef __RCREMOTEPROTOCOL_H__
#define __RCREMOTEPROTOCOL_H__

#include <RF24.h>

#include "rcSettings.h"
#include "rcGlobal.h"

#ifndef __RF24_H__
#error "rcRemoteProtocol Requires the tmrh20 RF24 Library: https://github.com/nRF24/RF24"
#endif


//Userdefined Constants
//Global constants can be found in rcGlobal.h


//Error constants
//Global constants can be found in rcGlobal.h

/**
 * Packet did not get to receiver
 */
#define RC_ERROR_PACKET_NOT_SENT -22
/**
 * The tick took longer than the wanted tick length.  See RCSettings.setCommsFrequency()
 */
#define RC_INFO_TICK_TOO_SHORT 21


/**
 * Communication Protocol for transmitters
 */
class RemoteProtocol : RCGlobal {
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
  RemoteProtocol(RF24* tranceiver, const uint8_t remoteId[]);

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
   * Check if the transmitter is connected with a receiver.
   *
   * @return true when connected.
   */
  bool isConnected();

  /**
   * Update the communications with the currently connected device
   *
   * This function holds until a specific amount of time has passed since
   * it was last called to fulfill RCSettings.setCommsFrequency()
   *
   * If telemetry is received from the receiver, telemetry will be updated, and
   * returns 1.
   *
   * @param channels array of size RCSettings.setNumChannels() to send
   * @param telemetry array of size RCSettings.setPayloadSize() to receive
   *
   * @return >= 0 if successful
   * @return 1 if telemetry was received
   * @return #RC_INFO_TICK_TOO_SHORT if RCSettings.setCommsFrequency() is
   * too high
   * @return #RC_ERROR_NOT_CONNECTED if there is no device connected
   * @return #RC_ERROR_PACKET_NOT_SENT
   */
  int8_t update(uint16_t channels[], uint8_t telemetry[]);

  /**
   * Get pointer for the current settings
   *
   * @return settings
   */
  RCSettings* getSettings();

private:

  const uint8_t* _remoteId;
  uint8_t _deviceId[5];

  //update variables
  bool _isConnected;
  uint32_t _timer;
  uint16_t _timerDelay;

  /**
   * Send a packet to the receiver
   *
   * When a packet is received, it will set returnData with the received data
   *
   * @param data data to write to receiver
   * @param dataSize size in bytes of data
   * @param telemetry data to be set if telemetry is received.
   * @param telemetrySize size in bytes of telemetry
   *
   * @return >= 0 if successfull
   * @return 1 if telemetry was updated
   * @return #RC_ERROR_PACKET_NOT_SENT
   * @return #RC_ERROR_NOT_CONNECTED
   */
  int8_t send_packet(void* data, uint8_t dataSize, void* telemetry,
                     uint8_t telemetrySize);

};

#endif
