#include "CanManager.h"

CanManager can;

CAN_STRUCT(TEMPERATURE, 100, 
  int32_t temp;
);

TEMPERATURE msg;

void setup() {
    Serial.begin(115200);
    can.init(GPIO_NUM_11, GPIO_NUM_12);
    msg.temp = 0;
}
void loop() {
    /*
    can.send(msg);
    msg.temp += 1; 
    delay(1000);
    */

    can_frame frameRecue;
    while (can.receive_can(&frameRecue)) {
        
        if (frameRecue.id == TEMPERATURE::ID) {
            TEMPERATURE msg_recu;
            memcpy(&msg_recu, frameRecue.data, sizeof(TEMPERATURE));

            Serial.print("Message ID 100 reçu : Temp = ");
            Serial.println(msg_recu.temp);
        }
    }
}