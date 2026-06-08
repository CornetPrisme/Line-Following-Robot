#include "CanManager.h"

CanManager can;

CAN_STRUCT(Data, 100, 
  int32_t temp;
);

Data msg;

void setup() {
    can.begin(GPIO_NUM_4, GPIO_NUM_5);
    msg.temp = 0;
}
void loop() {
    if (can.send_data(msg)) {
        Serial.print("message envoyé !");
    }

    msg.temp += 1; 
    delay(1000);
}