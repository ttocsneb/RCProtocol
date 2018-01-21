#ifndef __RCGLOBAL_H__
#define __RCGLOBAL_H__

#include <RF24.h>

#include "rcSettings.h"

//Global User defined constants

#ifndef RC_TIMEOUT
#define RC_TIMEOUT 15000
#endif

#ifndef RC_CONNECT_TIMEOUT
#define RC_CONNECT_TIMEOUT 2500
#endif

//Global Error Constants

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
 * Transmitter is already connected, so the function should not be run.
 */
#define RC_ERROR_ALREADY_CONNECTED -22

/**
 * Contains functions and variables used by both DeviceProtocal and
 * RemoteProtocol.
 *
 * Everything in RCGlobal is protected, so only DeviceProtocol and
 * RemoteProtocol have access to these members.
 *
 * You can also find Global constants in rcGlobal.h
 */
class RCGlobal {
protected:

  RCGlobal();

  const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};//Pair0: 0x50 61 69 72 30
  const uint8_t _DISCONNECT[5] = {0, 0, 0, 0, 0};//\0\0\0\0\0: 0x00 00 00 00 00
  const uint8_t _ACK = 0x06;
  const uint8_t _NACK = 0x15;
  const uint8_t _TEST = 0x02;

  const uint8_t _PACKET_CHANNELS = 0xA0;
  const uint8_t _PACKET_UPDATE_TRANS_SETTINGS = 0xB1;//TODO: Implement
  const uint8_t _PACKET_UPDATE_RECVR_SETTINGS = 0xB2;//TODO: Implement
  const uint8_t _PACKET_DISCONNECT = 0xC0;
  const uint8_t _PACKET_RECONNECT = 0xCA;

  RCSettings _settings;
  RCSettings _pairSettings;

  RF24* _radio;

  /**
   * repeatidly send a packet of buf until the packet has been received.
   *
   * @param buf data to send
   * @param size size of data in bytes
   * @param timeout how long before giving up. (millis)
   *
   * @return 0 if successful
   * @return -1 if not
   */
  int8_t force_send(void* buf, uint8_t size, unsigned long timeout);
  /**
   * Wait until a packet has been received
   *
   * @param timeout how long before giving up (millis)
   */
  int8_t wait_till_available(unsigned long timeout);

  /**
   * apply the given settings to the radio
   *
   * @param settings
   */
  void apply_settings(RCSettings* settings);

  /**
   * Flush the radio's input buffer
   */
  void flush_buffer();
};

#endif