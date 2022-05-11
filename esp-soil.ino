#include <Arduino.h>
#include <Wire.h>
#include "driver/adc.h"
#include "driver/ledc.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

String API_KEY      = "<YOUR-API-KEY>";
const char *MY_SSID = "<YOUR-SSID>"; 
const char *MY_PWD  = "<YOUR-PASSWORD>";

WiFiMulti wifiMulti;

const int PIN_LED   = 21;
const int PIN_CLK   = 17;
const int PIN_SOIL  = 9;

#define SLEEP_TIME 1200 * 1000 * 1000

#define NO_OF_SAMPLES   64          //Multisampling

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_DUTY_RES       LEDC_TIMER_3_BIT

#define LEDC_LS_CH0_GPIO       (17)
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0

// %50 duty: 2 ^ (LEDC_LS_DUTY_RES - 1) 
#define LEDC_TEST_DUTY          (4)

// I2C address for temperature sensor
const int TMP_ADDR  = 0x48;

const adc_channel_t      MOISTURE_CHANNEL    = ADC_CHANNEL_8;     // GPIO9
const adc_bits_width_t   WIDTH               = ADC_WIDTH_BIT_13;
const adc_atten_t        MOISTURE_ATTEN      = ADC_ATTEN_DB_2_5;

const adc_channel_t      BATTERY_CHANNEL     = ADC_CHANNEL_6;     // GPIO7
const adc_atten_t        BATTERY_ATTEN       = ADC_ATTEN_DB_11;

void connectWifi() {
    int retry = 0;
    Serial.print("Connecting to " + *MY_SSID);
    wifiMulti.addAP(MY_SSID, MY_PWD);

    while(wifiMulti.run() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");

        if(retry++ >= 10){
            WiFi.beginSmartConfig();
            while(1){
                delay(1000);
                if(WiFi.smartConfigDone()){
                    Serial.println("SmartConfig Success");
                    break;
                }
            }
        }
    }

    Serial.println("");
    Serial.println("Connected");
    Serial.println("");  
}

float readBattery() {
    uint32_t adc_reading = 0;

    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)BATTERY_CHANNEL);
    }
    adc_reading /= NO_OF_SAMPLES;

    return adc_reading;
}

uint32_t readMoisture() {
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)MOISTURE_CHANNEL);
    }
    adc_reading /= NO_OF_SAMPLES;
    return adc_reading;
}

float readTemp() {
    float temp;

    const int sdaPin = 8;
    const int sclPin = 10;
    Wire.begin(sdaPin, sclPin);

    Wire.beginTransmission(TMP_ADDR);
    // Select Data Registers
    Wire.write(0X00);
    Wire.endTransmission();

    delay(500);

    // Request 2 bytes , Msb first
    Wire.requestFrom(TMP_ADDR, 2 );
    // Read temperature as Celsius (the default)
    while(Wire.available()) {  
        int msb = Wire.read();
        int lsb = Wire.read();
        Wire.endTransmission();

        int rawtmp = msb << 8 |lsb;
        int value = rawtmp >> 4;
        temp = value * 0.0625;

        return temp;
    }

    return 0;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(PIN_LED, OUTPUT);

    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

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

    //Configure ADC
    adc1_config_width(WIDTH);
    adc1_config_channel_atten((adc1_channel_t)MOISTURE_CHANNEL, MOISTURE_ATTEN);
    adc1_config_channel_atten((adc1_channel_t)BATTERY_CHANNEL, BATTERY_ATTEN);

    connectWifi();
    delay(2000);
}

void sendData(float batt, float temp, float soil_hum) {  
    HTTPClient http;
    http.begin("http://api.thingspeak.com/update");
    http.addHeader("X-THINGSPEAKAPIKEY", API_KEY);

    String postStr = API_KEY;
    postStr += "&field1=";
    postStr += String(batt);
    postStr += "&field2=";
    postStr += String(temp);
    postStr += "&field3=";
    postStr += String(soil_hum);
    postStr += "\r\n\r\n";

    int httpCode = http.POST(postStr);
    http.end();

    Serial.printf("HTTP Response code=%d\n", httpCode);
}

void loop() {
    int soil_hum = readMoisture();
    float temp = readTemp();
    float batt = readBattery();
    Serial.printf("loop moisture: %d, temperature: %f batt: %f\n", soil_hum, temp, batt);
    sendData(batt, temp, soil_hum);
    // blink
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(300);                       // wait for a second
        digitalWrite(PIN_LED, LOW);    // turn the LED off by making the voltage LOW
        delay(300);
    }
    Serial.printf("go to sleep\n");
    ESP.deepSleep(SLEEP_TIME);
}

