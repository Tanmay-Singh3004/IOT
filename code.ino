#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include <Esp32Servo.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* apiEndpoint = "http://your.api.server/api";

#define SERVO1_PIN 12
#define SERVO2_PIN 13
#define SERVO3_PIN 14
#define SERVO4_PIN 15
#define SERVO5_PIN 16
#define SERVO6_PIN 17

Servo servo1, servo2, servo3, servo4, servo5, servo6;

void initCamera(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  esp_camera_init(&config);
}

void setup(){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED){ delay(500); }
  initCamera();
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);
  servo4.attach(SERVO4_PIN);
  servo5.attach(SERVO5_PIN);
  servo6.attach(SERVO6_PIN);
}

void applyIK(float x, float y, float z){
  float s1 = constrain(map(x, 0, 100, 0, 180), 0, 180);
  float s2 = constrain(map(y, 0, 100, 0, 180), 0, 180);
  float s3 = constrain(map(z, 0, 100, 0, 180), 0, 180);
  float s4 = 90;
  float s5 = 90;
  float s6 = 90;
  servo1.write(s1);
  servo2.write(s2);
  servo3.write(s3);
  servo4.write(s4);
  servo5.write(s5);
  servo6.write(s6);
  delay(2000);
}

void loop(){
  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb){ return; }
  HTTPClient http;
  http.begin(apiEndpoint);
  http.addHeader("Content-Type", "image/jpeg");
  int httpResponseCode = http.POST(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  if(httpResponseCode > 0){
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    float x = doc["x"];
    float y = doc["y"];
    float z = doc["z"];
    applyIK(x, y, z);
  }
  http.end();
  delay(5000);
}
