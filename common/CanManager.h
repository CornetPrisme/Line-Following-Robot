#pragma once
#include <Arduino.h>
#include "driver/twai.h"

struct CanFrame {
  uint32_t id = 0;       
  uint8_t data[8] = {0}; 
  uint8_t length = 0;    
  bool is_request = false;   
};