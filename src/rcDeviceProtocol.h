/*
  rcDeviceProtocol.h - Library for Device for RCProtocol.
  Created by Benjamin Jacobs, October 2, 2017
*/

#ifndef __RCDEVICEPROTOCOL_H__
#define __RCDEVICEPROTOCOL_H__

#include <Arduino.h>
#include <RF24.h>

#include "rcSettings.h"
#include "rcGlobal.h"

#ifndef __RF24_H__
#error "rcDeviceProtocol Requires the tmrh20 RF24 Library: https://github.com/nRF24/RF24"
#endif


//Userdefined Constants

#ifndef RC_TIMEOUT
#define RC_TIMEOUT 15000
#endif

#ifndef RC_CONNECT_TIMEOUT
#define RC_CONNECT_TIMEOUT 2500
#endif

//Error Constants

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
 * Transmitter refused to connect
 */
#define RC_ERROR_CONNECTION_REFUSED -14
/**
 * Receiver is not connected, so the function could not operate properly
 */
#define RC_ERROR_NOT_CONNECTED -21

/**
 * Communication Protocol for receivers
 */
class DeviceProtocol : RCGlobal {
public:
  /**
   * Save the transmitter id to non-volitile memory.
   * 
   * Very simple, all that needs to be done, is to save the Id to memory, 
   * so that when the receiver tries to connect, it will know which id to
   * request to connect with.
   * 
   * @param id 5 byte char array
   */
  typedef void (saveRemoteID)(const uint8_t* id);

  /**
   * Constructor
   * 
   * Creates a new instance of the protocol. You create an instance and send a reference
   * to the RF24 driver as well as the id of the remote
   * 
   * @param tranceiver A reference to the RF24 chip, this allows you to create your own instance,
   * allowing multi-platform support
   * @param deviceId The 5 byte char array of the receiver's ID: ex "MyRcr"
   */
  DeviceProtocol(RF24 *tranceiver, const uint8_t deviceId[]);

  /**
   * Begin the Protocol
   * 
   * @note There is no need to begin the RF24 driver, as this function already does this for you
   * 
   * @param settings RCSettings
   */
  void begin(RCSettings* settings);
  
  /**
   * Attempt to pair with a transmitter
   * 
   * @note The transmitter you are trying to pair with should also be in pair mode
   * 
   * @param saveRemoteID A function pointer to save the id of the transmitter.
   * 
   * @return 0 if successful
   * @return #RC_ERROR_TIMEOUT if no transmitter was found.
   * @return #RC_ERROR_LOST_CONNECTION if transmitter stopped replying
   */
  int8_t pair(saveRemoteID saveRemoteID);

  /**
   * Check if the receiver is connected with a transmitter.
   * 
   * @return true when connected.
   */
  bool isConnected();

  /**
   * Attempt to pair with a transmitter
   * 
   * @note The transmitter you are trying to connect with should also be in connect mode, 
   * as well as paired with this device
   * 
   * @param remoteId a 5 byte char array of the remote you are trying to pair with, this should
   * come from DeviceProtocol::saveRemoteID
   * 
   * @return 0 if successful
   * @return #RC_ERROR_TIMEOUT if no transmitter was found
   * @return #RC_ERROR_LOST_CONNECTION if transmitter stopped replying
   * @return #RC_ERROR_CONNECTION_REFUSED if the transmitter refused to connect
   * @return #RC_ERROR_BAD_DATA if the settings are not properly set, or 
   * the transmitter sent unexpected data
   */
  int8_t connect(uint8_t remoteId[]);

  /**
   * Set the Channel Array
   * 
   * This changes the memory address of the given pointer array to the 
   * array that gets received by the transmitter.
   * 
   * @note This needs to be set once per connect, because the 
   * pointer will change when a new connection is made.
   * 
   * Example:
   * 
   *     uint16_t* channels;
   *     uint8_t channelSize;
   *     channelSize = RCDevice.setChannelArray(&channels);
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
   * array that sends telemetry to the transmitter.
   * 
   * @note This needs to be set once per connect, because the 
   * pointer will change when a new connection is made.
   * 
   * Example:
   * 
   *     uint8_t* telemetry;
   *     uint8_t telemetrySize;
   *     telemetrySize = RCDevice.setTelemetryArray(&telemetry);
   * 
   * @param telemetry referenced pointer array to set
   * 
   * @return size of the array
   */
  uint8_t setTelemetryArray(uint8_t* &telemetry);

  /**
   * Update the communications with the currently connected device
   * 
   * If there was a packet sent, it will process it.
   * 
   * If the packet was a standard packet, it will set the channels array,
   * to whatever was received, and return 1
   * 
   * @note To get/send channels/telemetry Use setChannelArray() and 
   * setTelemetryArray(). You can access the data through the arrays you 
   * set.
   * 
   * @return 1 if channels was updated
   * @return 0 if nothing happened
   * @return #RC_ERROR_NOT_CONNECTED if not connected 
   */
  int8_t update();
private:
  //"Pair0" is not supported by the compiler for some reason, so an explicit array is used.
  const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
  const uint8_t _YES = 0x6; //ACKNOWLEDGE
  const uint8_t _NO = 0x15; //NEGATIVE ACKNOWLEDGE
  const uint8_t _TEST = 0x2; //Start Of Text
  
  const uint8_t _STDPACKET = 0xA0;//A0-AF are reserved for standard packets.

  RCSettings *_settings;
  RCSettings _pairSettings;

  RF24 *_radio;

  const uint8_t *_deviceId;
  uint8_t _remoteId[5];
  bool _isConnected;

  uint8_t* _telemetry;
  uint8_t _telemetrySize;
  uint16_t* _channels;
  uint8_t _channelsSize;

  int8_t _forceSend(void *buf, uint8_t size, unsigned long timeout);
  int8_t _waitTillAvailable(unsigned long timeout);

  /**
   * Check if a packet is available, and read it to returnData
   * 
   * @param returnData array of size RCSettings.setPayloadSize()
   * 
   * @return 1 if data available
   * @return 0 if nothing is available
   * @return #RC_ERROR_NOT_CONNECTED if not connected
   */
  int8_t _checkPacket(uint8_t *returnData);

  void _flushBuffer();

  void _applySettings(RCSettings *settings);

};

#endif
