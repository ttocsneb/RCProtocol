/*
   rcRemoteProtocol.h - Library for Remote for RCProtocol.
   Created by Benjamin Jacobs, October 1, 2017
*/

#ifndef __RCREMOTEPROTOCOL_H__
#define __RCREMOTEPROTOCOL_H__

#include <Arduino.h>
#include <RF24.h>

#include "rcSettings.h"
#include "rcGlobal.h"

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
#define RC_ERROR_LOST_CONNECTION -11
/**
 * No connection has been made
 */
#define RC_ERROR_TIMEOUT -12
/**
 * Data that was received does not match expectations
 */
#define RC_ERROR_BAD_DATA -13
/**
 * Receiver was refused to connect
 */
#define RC_ERROR_CONNECTION_REFUSED -14
/**
 * Transmitter is not connected, so the function could not operate properly
 */
#define RC_ERROR_NOT_CONNECTED -21
/**
 * Packet did not get to receiver
 */
#define RC_ERROR_PACKET_NOT_SENT -22
/**
 * Expected ack payload, but got a regular ack instead
 */
#define RC_INFO_NO_ACK_PAYLOAD 21
/**
 * The tick took longer than the wanted tick length.  See RCSettings.setCommsFrequency()
 */
#define RC_INFO_TICK_TOO_SHORT 22


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
   * @note The receiver should have already been paired with the remote, 
   * and in connect mode
   * 
   * @warning channel and telemetry arrays need to be reset with 
   * #setChannelArray()/#setTelemetryArray() after a connection is made.
   * 
   * @param checkIfValid A function pointer to check if the found device 
   * has been paired, and to load the settings
   * 
   * @return 0 if successful
   * @return #RC_ERROR_TIMEOUT if no receiver was found.
   * @return #RC_ERROR_LOST_CONNECTION if receiver stopped replying
   * @return #RC_ERROR_CONNECTION_REFUSED if the receiver is not on the 
   * pair list.
   * @return #RC_ERROR_BAD_DATA if the settings are not set properly on 
   * both devices
   */
  int8_t connect(checkIfValid checkIfValid);
  
  /**
   * Check if the transmitter is connected with a receiver.
   * 
   * @return true when connected.
   */
  bool isConnected();

  /**
   * Set the Channel Array
   * 
   * This changes the memory address of the given pointer array to the 
   * array that gets sent to the receiver.
   * 
   * @note This needs to be set once per connect, because the 
   * pointer will change when a new connection is made.
   * 
   * Example:
   * 
   *     uint16_t* channels;
   *     uint8_t channelSize;
   *     channelSize = RCRemote.setChannelArray(&channels);
   * 
   * @param channels referenced pointer array to set
   * 
   * @return size of the array
   */
  uint8_t setChannelArray(uint16_t* &channels);

  /**
   * Set the Telemetry Array
   * 
   * This changes the memory address of the given pointer array to the 
   * array that receives telemetry from the receiver.
   * 
   * @note This needs to be set once per connect, because the 
   * pointer will change when a new connection is made.
   * 
   * Example:
   * 
   *     uint8_t* telemetry;
   *     uint8_t telemetrySize;
   *     telemetrySize = RCRemote.setTelemetryArray(&telemetry);
   * 
   * @param telemetry referenced pointer array to set
   * 
   * @return size of the array
   */
  uint8_t setTelemetryArray(uint8_t* &telemetry);

  /**
   * Update the communications with the currently connected device
   * 
   * This function holds until a specific amount of time has passed since 
   * it was last called to fulfill RCSettings.setCommsFrequency()
   * 
   * It will also send channels.  In order to set the channels, see 
   * setChannelArray()
   * 
   * @return >= 0 if successful
   * @return #RC_INFO_NO_ACK_PAYLOAD if no ack payload was received
   * @return #RC_INFO_TICK_TOO_SHORT if RCSettings.setCommsFrequency() is 
   * too high
   * @return #RC_ERROR_NOT_CONNECTED if there is no device connected
   * @return #RC_ERROR_PACKET_NOT_SENT
   */
  int8_t update();

private:
  const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
  const uint8_t _YES = 0x6; //ACKNOWLEDGE
  const uint8_t _NO = 0x15; //NEGATIVE ACKNOWLEDGE
  const uint8_t _TEST = 0x2; //START OF TEXT

  const uint8_t _STDPACKET = 0xA0;//A0-AF are reserved for standard packets.


  const uint8_t *_remoteId;
  uint8_t _deviceId[5];

  RCSettings _settings;
  RCSettings _pairSettings;

  RF24 *_radio;

  //update variables
  bool _isConnected;
  uint32_t _timer;
  uint16_t _timerDelay;

  uint16_t* _channels;
  uint8_t _channelsSize;
  uint8_t* _telemetry;
  uint8_t _telemetrySize;

  /**
   * Send a packet to the receiver
   * 
   * When a packet is received, it will set returnData with the received data
   * 
   * @param data RCSettings.getPayloadSize() byte array to send
   * @param returnData RCSettings.getPayloadSize() byte array to receive
   * 
   * @return >= 0 if successfull
   * @return #RC_INFO_NO_ACK_PAYLOAD
   * @return #RC_ERROR_PACKET_NOT_SENT
   * @return #RC_ERROR_NOT_CONNECTED
   */
  int8_t _sendPacket(uint8_t* data, uint8_t* returnData);

  int8_t _forceSend(void *buf, uint8_t size, uint32_t timeout);
  int8_t _waitTillAvailable(uint32_t timeout);
  void _flushBuffer();

  void _applySettings(RCSettings *settings);
};

#endif
