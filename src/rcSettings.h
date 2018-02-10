#ifndef __RCSETTINGS_H__
#define __RCSETTINGS_H__

#include <RF24.h>

#define RC_TELEM_BATTERY     0b00000001
#define RC_TELEM_CURRENT     0b00000010
#define RC_TELEM_TEMPERATURE 0b00000100
#define RC_TELEM_RPM         0b00001000
#define RC_TELEM_GPS         0b00010000
#define RC_TELEM_ALARM1      0b00100000 //One Alarm
#define RC_TELEM_ALARM2      0b01000000 //Two Alarms
#define RC_TELEM_ALARM3      0b01100000 //Three Alarms

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
  void setSettings(const uint8_t* settings);
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
   * @warning DynamicPayloads are not currently implemented
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
  bool getEnableDynamicPayload() const;

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
  bool getEnableAck() const;

  /**
   * Enable/Disable Custom Ack Paylaods
   *
   * Custom ack payloads allow acknowledgments to be filled with
   * custom data. setEnableAck() needs to be enabled for this to work.
   *
   * @warning Disabling AckPayloads is not yet implemented
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
  bool getEnableAckPayload() const;

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
   * @param datarate see
   * [rf24_datarate_e](http://tmrh20.github.io/RF24/RF24_8h.html#a82745de4aa1251b7561564b3ed1d6522)
   */
  void setDataRate(rf24_datarate_e datarate);
  /**
   * Get the currently set value from setDataRate()
   *
   * @return datarate
   */
  rf24_datarate_e getDataRate() const;

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
  uint8_t getStartChannel() const;

  /**
   * Payload Size
   *
   * Set the size of the payload in bytes
   *
   * @note This can't be set higher than 32
   *
   * @warning Values other than 32 are not fully implemented, use at your own risk!
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
  uint8_t getPayloadSize() const;

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
   *
   * @return frequency
   */
  uint8_t getCommsFrequency() const;

  /**
   * Retry Delay
   *
   * Set the time before the transmission is resent.
   *
   * @note The shorter the delay, the smaller the ack packet can be.
   * see the table below for more information
   *
   * (x): time value
   *
   * | DataRate | Min Time for Full Packet | Ack Size at 250 us (0)                |
   * | -------- | ------------------------ | ------------------------------------- |
   * | 2MBPS    | 500 us (1)               | 15 bytes                              |
   * | 1MBPS    | 500 us (1)               | 5 bytes                               |
   * | 250KBPS  | 1500 us (5)              | None (500us (1) min for empty packet) |
   *
   * Uses `0b00001111` of byte 4
   *
   * Default: 15
   *
   * @param time How long to wait between each retry, in multiples of 250us, max is 15. 0 means
   * 250us, 15 means 4000us.
   */
  void setRetryDelay(uint8_t time);
  /**
   * Get the currently set value from setRetryDelay()
   *
   * @return retryDelay
   */
  uint8_t getRetryDelay() const;

  /**
   * Number of channels in a packet
   *
   * Set the number of channels in a packet, each channel uses 2 bytes
   * default, so a 32 byte packet can hold up to 15 channels.
   *
   * @note In the future, I may support multi-packet transmissions to allow more
   * channels.
   *
   * Uses byte 5
   *
   * Default: 6
   *
   * @param numChannels number of channels to send (1 to 15)
   */
  void setNumChannels(uint8_t numChannels);
  /**
   * Get the currently set value from setNumChannels()
   *
   * @return numChannels
   */
  uint8_t getNumChannels() const;

  /**
   * Set the telemetry bits to enable certain telemetry channels
   *
   * Use RC_TELEM_x | RC_TELEM_b to enable specific telemetry channels
   *
   * Uses byte 6
   *
   * Default: 0
   *
   * @param telemetryChannels
   *
   */
  void setTelemetryChannels(uint8_t telemetryChannels);
  /**
   * Get the value from setTelemetryChannels()
   *
   * @return telemetryChannels
   */
  uint8_t getTelemetryChannels() const;

  /**
   * Print the settings to Serial.
   */
  void printSettings() const;

private:
  uint8_t _settings[32];
};

#endif