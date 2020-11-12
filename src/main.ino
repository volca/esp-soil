#include <Wire.h>

const int PIN_CLK   = 17;
const int PIN_SOIL  = 9;

void setup() {
    Serial.begin(115200);
    delay(10);

    ledcAttachPin(PIN_CLK, 1);
    ledcAttachPin(PIN_SOIL, 2);
    ledcSetup(1, 12000, 8);
    ledcWrite(PIN_CLK, 128);
    delay(50);
}

void loop() {
    ledcWrite(PIN_CLK, 128);
    delay(50);
    float rawVal = ledcRead(2);
    Serial.printf("loop %f\n", rawVal);
    delay(1000);
}
