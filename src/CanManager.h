#pragma once
#include <Arduino.h>

#ifdef ARDUINO_ARCH_STM32
#include "STM32_CAN.h"
#endif
#ifdef ARDUINO_ARCH_ESP32
#include "driver/twai.h"
#endif

#define CALLBACKS_NUMBER 10 

struct can_frame_t {
  uint32_t id = 0;       
  uint8_t data[8] = {0}; 
  uint8_t length = 0;    
  bool rtr = false;   
};

constexpr uint32_t DUPLICATE_IDS[] = { 
    100,
    101,
    102,
    103    
};
constexpr bool is_duplicate_id(uint32_t id) {
    for (uint32_t duplicate_id : DUPLICATE_IDS) {
        if (duplicate_id == id) {
            return true;
        }
    }
    return false;
}

#define CAN_STRUCT(struct_name, can_id, ...) \
  struct __attribute__((packed)) struct_name { \
    static constexpr uint32_t ID = can_id; \
    __VA_ARGS__ \
  }; \
  static_assert(sizeof(struct_name) <= 8, #struct_name " exceeds CAN frame size (8 bytes)"); \
  static_assert(std::is_trivially_copyable<struct_name>::value, #struct_name " must be trivially copyable"); \
  static_assert(!is_duplicate_id(can_id), #can_id " is already in use"); \


class CanManager {
  public:

  #ifdef ARDUINO_ARCH_ESP32
  bool init(gpio_num_t rxpin, gpio_num_t txpin) {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(txpin, rxpin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        Serial.println("Driver installed");
    } else {
        Serial.println("Failed to install driver");
        return false;
    }

    // Start TWAI driver
    if (twai_start() == ESP_OK) {
        Serial.println("Driver started");
    } else {
        Serial.println("Failed to start driver");
        return false;
    }
    return true;
  };

  void send_can(can_frame_t frame) {
      twai_message_t message;
      message.identifier = frame.id;
      message.data_length_code = frame.length;
      memcpy(&message.data, frame.data, frame.length);
      if (twai_transmit(&message, 0) == ESP_OK) {
          printf("Message queued for transmission\n");
      } else {
          printf("Failed to queue message for transmission\n");
      }
  };


  bool receive_can(can_frame_t* frame) {
    twai_message_t message;
    if (twai_receive(&message, 0) == ESP_ERR_TIMEOUT) {
        // Serial.println("no can frames");
        return false;
    }
    frame->rtr = message.rtr;
    frame->length = message.data_length_code;
    frame->id = message.identifier;
    memcpy(frame->data, &message.data, message.data_length_code);
    return true;
  };
  #endif

  #ifdef ARDUINO_ARCH_STM32
  CanManager(): m_stm32CAN(CAN1, DEF) {};
  void init() {
    m_stm32CAN.begin();
    m_stm32CAN.setBaudRate(500000);
  };

  void send_can(can_frame_t frame) {
    CAN_message_t tx_msg;
    tx_msg.id = frame.id;
    tx_msg.len = frame.length;
    tx_msg.flags.remote = frame.rtr;
    memcpy(&tx_msg.buf, frame.data, frame.length);
    if (m_stm32CAN.write(tx_msg)) {
    };
  };

  bool receive_can(can_frame_t* frame) {
    CAN_message_t rx_msg;
    if (!m_stm32CAN.read(rx_msg)){
        return false;
    }

    frame->id = rx_msg.id;
    frame->length = rx_msg.len;
    frame->rtr = rx_msg.flags.remote;
    memcpy(frame->data, &rx_msg.buf, rx_msg.len);
    return true;
  };

#endif

  template<typename T>
  void send(T data) {
    can_frame_t frame {};
    frame.id = T::ID;
    frame.length = sizeof(data);
    memcpy(&frame.data, &data, frame.length);
    send_can(frame);
  }

  typedef void (*CanCallback)(can_frame_t);

  bool onReceive(uint32_t id, CanCallback funcToCall) {
      if (_callback_count < CALLBACKS_NUMBER) {
          _callbacks[_callback_count].id = id;
          _callbacks[_callback_count].function = funcToCall;
          _callback_count++;
          return true;
      }
      Serial.println("Trop de callbacks enregistrés (Max 10)"); // faire vérification automatique
      return false;
  }

  void update() {
      can_frame_t frame;
      while (this->receive_can(&frame)) {
          for (uint8_t i = 0; i < _callback_count; i++) {
              if (_callbacks[i].id == frame.id) {
                  _callbacks[i].function(frame);
              }
          }
      }
    }

  private:
  struct CallbackEntry {
    uint32_t id = 0;
    CanCallback function;
  };

  CallbackEntry _callbacks[CALLBACKS_NUMBER];
  uint8_t _callback_count = 0;

  #ifdef ARDUINO_ARCH_STM32
    STM32_CAN m_stm32CAN;
  #endif
};