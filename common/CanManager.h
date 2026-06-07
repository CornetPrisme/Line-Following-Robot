#pragma once
#include <Arduino.h>
#include "driver/twai.h"

struct CanFrame {
  uint32_t id = 0;       
  uint8_t data[8] = {0}; 
  uint8_t length = 0;    
  bool is_request = false;   
};

class CanManager {
  public:

  bool begin(gpio_num_t rxpin, gpio_num_t txpin) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txpin, rxpin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
    if (twai_start() == ESP_OK) {
        return true;
      } else {
        Serial.println("Failed to start driver");
        return false;
      }
  } else {
    Serial.println("Failed to install driver");
    return false;
  }
  }
  bool send(CanFrame frame) {
    twai_message_t message;
    message.identifier = frame.id;
    message.data_length_code = frame.length;
    message.rtr = frame.is_request;
    memcpy(message.data, frame.data, frame.length);
    return (twai_transmit(&message, pdMS_TO_TICKS(100)) == ESP_OK);
  }

  bool receive(CanFrame& frame) {
    twai_message_t message;
    if (twai_receive(&message, 0) == ESP_OK) {
      frame.id = message.identifier;
      frame.length = message.data_length_code;
      frame.is_request = message.rtr;
      memcpy(frame.data, message.data, message.data_length_code);
      return true; 
    }
    return false; 
  }
  
  template <typename T>
  bool sendData(uint32_t id, T data) {
    if (sizeof(T) > 8){
      Serial.println("Packet size is over 8 bytes");
      return false; 
    }
    CanFrame frame;
    frame.id = id;
    frame.length = sizeof(T);
    frame.is_request = false;
    memcpy(frame.data, &data, frame.length);
    return this->send(frame);
  }

  template <typename T>
  T extractData(CanFrame frame) {
    T data; 
    memcpy(&data, frame.data, frame.length); 
    return data;
  }
  };