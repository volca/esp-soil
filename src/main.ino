#include <Arduino.h>
#include <Wire.h>
#include "driver/adc.h"
#include "driver/ledc.h"
#include "WiFi.h"
#include <WiFiMulti.h>
#include <FFat.h>

FS* filesystem =      &FFat;
#define FileFS        FFat
#define FS_Name       "FFat"

WiFiMulti wifiMulti;

const int PIN_CLK   = 17;
const int PIN_SOIL  = 9;

const char* MY_SSID = "siyue2L-dong"; 
const char* MY_PWD = "siyuexiongdi";

#define NO_OF_SAMPLES   64          //Multisampling

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_DUTY_RES       LEDC_TIMER_3_BIT

#define LEDC_LS_CH0_GPIO       (17)
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0

// %50 duty: 2 ^ (LEDC_LS_DUTY_RES - 1) 
#define LEDC_TEST_DUTY          (4)

#define DEFAULT_VREF            1100

// I2C address for temperature sensor
const int TMP_ADDR  = 0x48;

const adc_channel_t      MOISTURE_CHANNEL    = ADC_CHANNEL_8;     // GPIO9
const adc_bits_width_t   WIDTH               = ADC_WIDTH_BIT_13;
const adc_atten_t        MOISTURE_ATTEN      = ADC_ATTEN_DB_2_5;

const adc_channel_t      BATTERY_CHANNEL     = ADC_CHANNEL_6;     // GPIO7
const adc_atten_t        BATTERY_ATTEN       = ADC_ATTEN_DB_11;

#define SSID_MAX_LEN            32
#define PASS_MAX_LEN            64

#define NUM_WIFI_CREDENTIALS      2
#define  CONFIG_FILENAME              F("/wifi_cred.dat")

#define ESP_getChipId()   ((uint32_t)ESP.getEfuseMac())

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
} WM_Config;

WM_Config         WM_config;

#define USE_AVAILABLE_PAGES     false

#define USE_ESP_WIFIMANAGER_NTP     false

#include <ESP_WiFiManager.h>

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
}

void connectWifi() {
  Serial.print("Connecting to " + *MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("Connected");
  Serial.println("");  
}

void setup() {
    Serial.begin(115200);
    delay(10);

    // Format FileFS if not yet
    if (!FileFS.begin(true)) {
        Serial.print(FS_Name);
        Serial.println(F(" failed! AutoFormatting."));
    }

    ESP_WiFiManager ESP_wifiManager("APlant");
    ESP_wifiManager.setDebugOutput(true);

    String chipID = String(ESP_getChipId(), HEX);
    chipID.toUpperCase();

    // SSID and PW for Config Portal
    String AP_SSID = "ESP_" + chipID + "_AutoConnectAP";
    String AP_PASS = "MyESP_" + chipID;

    if ( !ESP_wifiManager.startConfigPortal(AP_SSID.c_str(), AP_PASS.c_str()) )
      Serial.println("Not connected to WiFi but continuing anyway.");
    else
      Serial.println("WiFi connected...yeey :)");

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
}

void loop() {
    int adc_reading = readMoisture();
    float temp = readTemp();
    float batt = readBattery();
    Serial.printf("loop moisture: %d, temperature: %f batt: %f\n", adc_reading, temp, batt);
    delay(1000);
}
