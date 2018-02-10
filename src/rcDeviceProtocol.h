/*
  rcDeviceProtocol.h - Library for Device for RCProtocol.
  Created by Benjamin Jacobs, October 2, 2017
*/

#ifndef __RCDEVICEPROTOCOL_H__
#define __RCDEVICEPROTOCOL_H__

#include <RF24.h>

#include "rcSettings.h"
#include "rcGlobal.h"
#include "rcDeviceInterface.h"
#include "rcTelemetry.h"

#ifndef __RF24_H__
#error "rcDeviceProtocol Requires the tmrh20 RF24 Library: https://github.com/nRF24/RF24"
#endif

//Userdefined Constants
//Global constants can be found in rcGlobal.h

//Error Constants
//Global constatns can be found in rcGlobal.h

/**
 * Communication Protocol for receivers
 */
class DeviceProtocol : RCGlobal {
public:

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
  DeviceProtocol(RF24* tranceiver, const uint8_t deviceId[]);

  /**
   * Begin the Protocol
   *
   * If the system power cycled or reset while in use, it will try to reconnect
   * immidiately.
   *
   * @note There is no need to begin the RF24 driver, as this function already
   * does this for you
   *
   * @param settings RCSettings
   * @param functions User-defined functions from #DeviceInterface
   *
   * @return 0
   * @return 1 if reconnected to remote
   * @return -1 if unable to reconnect to remtoe
   */
  int8_t begin(RCSettings* settings, DeviceInterface* functions);

  /**
   * Attempt to pair with a transmitter
   *
   * @note The transmitter you are trying to pair with should also be in pair mode
   *
   * @return 0 if successful
   * @return #RC_ERROR_ALREADY_CONNECTED if already connected to remote
   * @return #RC_ERROR_TIMEOUT if no transmitter was found.
   * @return #RC_ERROR_LOST_CONNECTION if transmitter stopped replying
   */
  int8_t pair();

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
   * @return 0 if successful
   * @return #RC_ERROR_ALREADY_CONNECTED if already connected to remote
   * @return #RC_ERROR_TIMEOUT if no transmitter was found
   * @return #RC_ERROR_LOST_CONNECTION if transmitter stopped replying
   * @return #RC_ERROR_CONNECTION_REFUSED if the transmitter refused to connect
   * @return #RC_ERROR_BAD_DATA if the settings are not properly set, or
   * the transmitter sent unexpected data
   */
  int8_t connect();

  /**
   * Update the communications with the currently connected device
   *
   * If there was a packet sent, it will process it.
   *
   * @param channels RCSettings.setNumChannels() size array that is set
   * when a standard packet is received.
   * @param telemetry
   *
   * @return 1 if channels were updated
   * @return 0 if nothing happened
   * @return #RC_ERROR_NOT_CONNECTED if not connected
   */
  int8_t update(uint16_t channels[], RCTelemetry* telemetry);

  /**
   * Get pointer for the current settings
   *
   * @return settings
   */
  RCSettings* getSettings();
private:

  const uint8_t* _deviceId;
  uint8_t _remoteId[5];
  bool _isConnected;
  DeviceInterface* _deviceFunctions;


  /**
   * Check if a packet is available, and read it to returnData
   *
   * @param returnData data to set if data was received
   * @param dataSize size of returnData in bytes
   * @param telemetry Data to Send back.  Note: telemetry won't be sent if
   * ack payloads are disabled
   * @param telemetrySize size of telemetry in bytes
   *
   * @return 1 if data is available
   * @return 0 if nothing is available
   * @return #RC_ERROR_NOT_CONNECTED if not connected
   */
  int8_t check_packet(void* returnData, uint8_t dataSize, void* telemetry,
                      uint8_t telemetrySize);
  int8_t check_packet(void* returnData, uint8_t dataSize);

};

#endif
