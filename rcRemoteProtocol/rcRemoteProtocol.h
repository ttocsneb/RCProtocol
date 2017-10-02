/*
 vim: tabstop=2 
 vim: softtabstop=0
 vim: expandtab
 vim: shiftwidth=2
 vim: smarttab
*/

/*
   rcRemoteProtocol.h - Library for Remote for RCProtocol.
   Created by Benjamin Jacobs, October 1, 2017
*/

#ifndef __RCREMOTEPROTOCOL_H__
#define __RCREMOTEPROTOCOL_H__

#include "RF24.h"
#include "Arduino.h"


//Userdefined Constants

#ifndef RC_TIMEOUT
#define RC_TIMEOUT 15000
#endif

#ifndef RC_CONNECT_TIMEOUT
#define RC_CONNECT_TIMEOUT 2500
#endif

//Error constants

#define RC_ERROR_LOST_CONNECTION -1
#define RC_ERROR_TIMEOUT -2

class RemoteProtocol {
  public:
    RemoteProtocol(RF24 *tranceiver, uint8_t remoteId[]);
    void begin();
    int8_t pair();
    int8_t connect();
    int8_t update();
  private:
    const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
    const uint8_t _YES = 'Y';
    const uint8_t _NO = 'N';

    uint8_t *_remoteId;
    RF24 *_radio;

    int8_t _forceSend(void *buf, uint8_t size, long timeout);
    int8_t _waitTillAvailable(long timeout);
};

#endif
