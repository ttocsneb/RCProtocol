/*
 vim: tabstop=2
 vim: softtabstop=0
 vim: expandtab
 vim: shiftwidth=2
 vim: smarttab
*/

/*
  rcDeviceProtocol.h - Library for Device for RCProtocol.
  Created by Benjamin Jacobs, October 2, 2017
*/

#ifndef __RCDEVICEPROTOCOL_H__
#define __RCDEVICEPROTOCOL_H__

#include "Arduino.h"
#include "RF24.h"

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

#define RC_ERROR_LOST_CONNECTION 1
#define RC_ERROR_TIMEOUT 2
#define RC_ERROR_CONNECTION_REFUSED 3
#define RC_ERROR_CONNECTION_BAD_DATA 4

class DeviceProtocol {
  public:
    DeviceProtocol(RF24 *tranceiver, const uint8_t deviceId[]);
    void begin();
    int8_t pair();
    int8_t connect(uint8_t remoteId[]);
    int8_t update();
  private:
    //"Pair0" is not supported by the compiler for some reason, so an explicit array is used.
    const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
    const uint8_t _YES = 'Y';
    const uint8_t _NO = 'N';

    const uint8_t *_deviceId;
    RF24 *_radio;

    int8_t _forceSend(void *buf, uint8_t size, long timeout);
    int8_t _waitTillAvailable(long timeout);

};

#endif
