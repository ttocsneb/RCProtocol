
#ifndef __RCGLOBAL_H__
#define __RCGLOBAL_H__

#include <RF24.h>

#include "rcSettings.h"

class RCGlobal {
protected:

  RCGlobal();

  const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
  const uint8_t _ACK = 0x06;
  const uint8_t _NACK = 0x15;
  const uint8_t _TEST = 0x02;

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