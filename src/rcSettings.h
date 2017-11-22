#ifndef __RCSETTINGS_H__
#define __RCSETTINGS_H__

#include <RF24.h>

class RCSettings {
public:
  /**
   * Create a new instance of RCSettings.
   * 
   * When you create RCSettings all of the 
   * default values will be set.
   * 
   * This object is used by DeviceProtocol::begin()
   */
  RCSettings();

  /**
   * Set all of the settings at once with one array of settings.
   * 
   * @param settings 32 byte array
   */ 
  void setSettings(const uint8_t *settings);
  /**
   * Get all of the settings as one array of settings
   * 
   * @return 32 byte array
   */
  uint8_t* getSettings();


  /**
   * Enable/Disable Dynamic payloads
   * 
   * Dynamic payloads don't have a specific payload size, and will
   * vary from transaction to transaction.
   * 
   * If you disable dynamic payloads, see setPayloadSize()
   * 
   * Uses `0b00000001` of byte 0
   * 
   * Default: false
   * 
   * @param enable
   */
  void setEnableDynamicPayload(bool enable);
  /**
   * get the currently set value from setEnableDynamicPayload()
   * 
   * @return enable dynamic payload
   */
  bool getEnableDynamicPayload();

  /**
   * Enable/Disable Acknowledgements
   * 
   * Acknowledgements are used so the sender can know if
   * the receiver received the message.
   * 
   * You can disable this if you want but that comes at the cost of
   * not getting telemetry data.
   * 
   * Uses `0b00000010` of byte 0
   * 
   * Default: true
   * 
   * @param enable
   */
  void setEnableAck(bool enable);
  /**
   * get the currently set value from setEnableAck()
   * 
   * @return enable acknowledgements
   */
  bool getEnableAck();

  /**
   * Enable/Disable Custom Ack Paylaods
   * 
   * Custom ack payloads allow acknowledgments to be filled with
   * custom data. setEnableAck() needs to be enabled for this to work.
   * 
   * This should be set to true to enable telemetry data.
   * 
   * Uses `0b00000100` 0f byte 0
   * 
   * Default: true
   * 
   * @note once this is set to true, it can not be turned off 
   * until the next power cycle.  Though not much is affected with
   * it enabled.
   * 
   * @param enable
   */
  void setEnableAckPayload(bool enable);
  /**
   * Get the currently set value from setEnableAckPayload()
   * 
   * @return enable ack payloads
   */
  bool getEnableAckPayload();
  
  /**
   * DataRate
   * 
   * There are two options for datarate, three if you have a (+) model
   * 
   *   - RF24_1MBPS
   *   - RF24_2MBPS
   *   - RF24_250KBPS (+)
   * 
   * Uses `0b00011000` of byte 0
   * 
   * Default: RF24_1MBPS
   * 
   * @param datarate see [rf24_datarate_e](http://tmrh20.github.io/RF24/RF24_8h.html#a82745de4aa1251b7561564b3ed1d6522)
   */
  void setDataRate(rf24_datarate_e datarate);
  /**
   * Get the currently set value from setDataRate()
   * 
   * @return datarate
   */
  rf24_datarate_e getDataRate();

  /**
   * Starting radio Channel.
   * 
   * There are 128 channels (0-127) to choose from.  Selecting an
   * arbitrary number can help lower the chances of interference.
   * 
   * Uses byte 1
   * 
   * Default: 0
   * 
   * @param channel 0-127
   */
  void setStartChannel(uint8_t channel);
  /**
   * Get the currently set value from setStartChannel()
   * 
   * @return channel
   */
  uint8_t getStartChannel();

  /**
   * Payload Size
   * 
   * Set the size of the payload in bytes
   * 
   * Normally this isn't set higher than 32
   * 
   * Uses byte 2
   * 
   * Default: 32
   * 
   * @param payload
   */
  void setPayloadSize(uint8_t payload);
  /**
   * Get the currently set value from setPayloadSize()
   * 
   * @return payload
   */
  uint8_t getPayloadSize();

  /**
   * Communication Frequency
   * 
   * Set the number of transactions the transmitter will send per second.
   * The max frequency is 255hz.
   * 
   * Uses byte 3
   * 
   * Default: 60
   * 
   * @note the actual frequency will become more innacurate as the set
   * frequency is increased. 
   * 
   */
  void setCommsFrequency(uint8_t frequency);
  /**
   * Get the currently set value from setCommsFrequency()
   */
  uint8_t getCommsFrequency();

private:
  uint8_t _settings[32];
};

#endif