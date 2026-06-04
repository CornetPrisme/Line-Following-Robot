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
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx_pin, rx_pin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("Driver installed");
  } else {
    Serial.println("Failed to install driver");
    return;
  }

  
};