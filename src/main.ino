#include <Wire.h>
//#include "driver/adc.h"
#include "driver/ledc.h"

const int PIN_CLK   = 17;
const int PIN_SOIL  = 9;

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_DUTY_RES       LEDC_TIMER_3_BIT

#define LEDC_LS_CH0_GPIO       (17)
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0

// %50 duty: 2 ^ (LEDC_LS_DUTY_RES - 1) 
#define LEDC_TEST_DUTY          (4)

void setup() {
    Serial.begin(115200);
    delay(10);

    // ledc
    // Set configuration of timer0 for high speed channels
    /*
     * Prepare and set configuration of timers
     * that will be used by LED Controller
     */
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LS_MODE,           // timer mode
        .duty_resolution = LEDC_TIMER_3_BIT, // resolution of PWM duty
        .timer_num = LEDC_LS_TIMER,            // timer index
        .freq_hz = 8000000,                      // frequency of PWM signal
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num   = LEDC_LS_CH0_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .channel    = LEDC_LS_CH0_CHANNEL,
        .timer_sel  = LEDC_LS_TIMER,
        .duty       = LEDC_TEST_DUTY,
        .hpoint     = 0,
    };
    ledc_channel_config(&ledc_channel);

    analogReadResolution(13);
    analogSetAttenuation(ADC_2_5db);
}

void loop() {
    delay(50);
    float adc_reading = analogRead(PIN_SOIL);
    Serial.printf("loop %f\n", adc_reading);
    delay(1000);
}
