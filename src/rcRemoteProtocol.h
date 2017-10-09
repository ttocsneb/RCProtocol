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

#define RC_ERROR_LOST_CONNECTION -1
#define RC_ERROR_TIMEOUT -2

class RemoteProtocol {
  public:
    RemoteProtocol(RF24 *tranceiver, const uint8_t remoteId[]);
    void begin();
    //The function pointer saves the settings from the device we have just paired with,
    //it has two arguments:
    //  uint8_t *id: a char array that will always be 5 characters long
    //  uint8_t *settings: a char arrray that holds the settings
    //  uint8_t settingsSize: the size of the settings array
    int8_t pair(void (saveSettings)(uint8_t*, uint8_t*, uint8_t));
    //The function pointer should check if the device has been paired, and if its required
    //are present.
    //  uint8_t *id: a char array that will always contain 5 characters.
    int8_t connect(bool (*checkIfValid)(uint8_t*));
    int8_t update();
  private:
    const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
    const uint8_t _YES = 'Y';
    const uint8_t _NO = 'N';

    const uint8_t *_remoteId;

    RF24 *_radio;

    int8_t _forceSend(void *buf, uint8_t size, long timeout);
    int8_t _waitTillAvailable(long timeout);
};

#endif
