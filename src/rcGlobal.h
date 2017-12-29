#ifndef __RCGLOBAL_H__
#define __RCGLOBAL_H__

#include <Arduino.h>

class RCGlobal {
protected:

  /**
   * Copy one array to another.
   * 
   * This can accept any type of value, but it will copy them as if they 
   * are ints.
   * 
   * @param destination pointer to array that will be copied to
   * @param start pointer to array that will be copied
   * @param size the size of the arrays in bytes (use sizeof())
   */
  void rc_cpy(void* destination, const void* start, uint8_t size);

};

#endif