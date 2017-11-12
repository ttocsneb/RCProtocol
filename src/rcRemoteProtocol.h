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
#define RC_ERROR_BAD_DATA -3

//Settings positions
#define SET_BOOLS 0
#define SET_ENABLE_DYNAMIC_PAYLOAD(x, y) (x==true?(y|1):(y&(~1)))
#define GET_ENABLE_DYNAMIC_PAYLOAD(x) (x&1)
#define SET_ENABLE_ACK(x, y) (x==true?(y|2):(y&(~2)))
#define GET_ENABLE_ACK(x) ((x>>1)&1)
#define SET_ENABLE_ACK_PAYLOAD(x, y) (x==true?(y|4):(y&(~4)))
#define GET_ENABLE_ACK_PAYLOAD(x) ((x>>2)&1)

#define SET_START_CHANNEL 1
#define SET_DATA_RATE 2
#define SET_PAYLOAD_SIZE 3

class RemoteProtocol {
  public:
    RemoteProtocol(RF24 *tranceiver, const uint8_t remoteId[]);
    void begin();
    //The function pointer saves the settings from the device we have just paired with,
    //it has two arguments:
    //  const uint8_t *id: a char array that will always be 5 characters long
    //  cosnt uint8_t *settings: a 32 byte arrray that holds the settings
    int8_t pair(void (saveSettings)(const uint8_t*, const uint8_t*));
    //The function pointer should check if the device has been paired, and if its required
    //are present.
    //  const uint8_t *id: a char array that will always contain 5 characters.
    //  uint8_t *settings: an empty 32 byte array that should be populated with the settings of the ID
    int8_t connect(bool (checkIfValid)(const uint8_t*, uint8_t*));
    int8_t update();
  private:
    const uint8_t _PAIR_ADDRESS[5] = {'P', 'a', 'i', 'r', '0'};
    const uint8_t _YES = 0x6; //ACKNOWLEDGE
    const uint8_t _NO = 0x15; //NEGATIVE ACKNOWLEDGE
    const uint8_t _TEST = 0x2; //START OF TEXT


    const uint8_t *_remoteId;
    uint8_t _settings[32];

    RF24 *_radio;

    int8_t _forceSend(void *buf, uint8_t size, uint32_t timeout);
    int8_t _waitTillAvailable(uint32_t timeout);
    void _flushBuffer();
};

#endif
