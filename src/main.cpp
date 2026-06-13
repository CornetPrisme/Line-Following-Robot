#include "CanManager.h"

CanManager can;

CAN_STRUCT(TEMPERATURE, 106, 
  int32_t temp;
);

TEMPERATURE msg;

void envoie(can_frame_t frameRecue) {
    can.send(msg);
    msg.temp += 1; 
    delay(1000);
}

void reception(can_frame_t frameRecue) {
    while (can.receive_can(&frameRecue)) {
        
        if (frameRecue.id == TEMPERATURE::ID) {
            TEMPERATURE msg_recu;
            memcpy(&msg_recu, frameRecue.data, sizeof(TEMPERATURE));

            Serial.print("Message ID 100 reçu : Temp = ");
            Serial.println(msg_recu.temp);
        }
    }
}



void setup() {
    Serial.begin(115200);

    #ifdef ARDUINO_ARCH_ESP32
      can.init(GPIO_NUM_11, GPIO_NUM_12);
    #endif
    #ifdef ARDUINO_ARCH_STM32
      can.init();
    #endif
    
    msg.temp = 0;
}
void loop() {
    can_frame_t frameRecue;
    //envoie(frameRecue);
    reception(frameRecue);
}