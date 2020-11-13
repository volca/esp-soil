#include <Wire.h>
#include "driver/adc.h"

const int PIN_CLK   = 17;
const int PIN_SOIL  = 9;

#define NO_OF_SAMPLES   64          //Multisampling

static const adc_channel_t channel = ADC_CHANNEL_8;     // GPIO7 if ADC1, GPIO17 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_13;
static const adc_atten_t atten = ADC_ATTEN_DB_2_5;
static const adc_unit_t unit = ADC_UNIT_1;

void setup() {
    Serial.begin(115200);
    delay(10);

    ledcAttachPin(PIN_CLK, 1);
    //ledcAttachPin(PIN_SOIL, 2);
    ledcSetup(1, 8000000, 3);
    ledcWrite(PIN_CLK, 4);
    delay(50);

    //Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten((adc1_channel_t)channel, atten);
}

void loop() {
    delay(50);
    //float rawVal = analogRead(PIN_SOIL);

    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)channel);
    }
    adc_reading /= NO_OF_SAMPLES;

    Serial.printf("loop %d\n", adc_reading);
    delay(1000);

}
