#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "esp_camera.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22


// Global variables field
namespace {
  // Wifi connection parameters
  const char *SSID = "HUAWEI P smart Pro";
  const char *PWD = "03ac5f40";

  // Web server
  AsyncWebServer server(80); // AsyncWebServer instance for handling HTTP requests

  // Image data and parameters
  camera_fb_t * fb = NULL;
  int pictureNumber = 2;

  const int picture_delay = 500;

}


struct CaptureInfo
{
  int pictureNum = -1;
  String picturePath = "";
  String error = "";
};


bool take_picture(CaptureInfo* info)
{
  fb = esp_camera_fb_get();
  if (!fb) {
    info->error = "Camera capture failed";
    return false;
  }

  info->pictureNum = pictureNumber % 256; // looping last 256 photos

  // Path where new picture will be saved in SD Card
  info->picturePath = "/picture" + String(info->pictureNum) + ".jpg";

  fs::FS &fs = SD_MMC;
  Serial.printf("Picture file name: %s\n", info->picturePath.c_str());

  File file = fs.open(info->picturePath.c_str(), FILE_WRITE);
  if (!file) {
    info->error = "Failed to open file in writing mode";
    return false;
  }
  else
  {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", info->picturePath.c_str());
    pictureNumber++;
  }
  file.close();
  esp_camera_fb_return(fb);

  delay(picture_delay);

  return true;
}


void setup_routing()
{
  server.on("/data", HTTP_GET, getData);
  server.begin();
}


void getData(AsyncWebServerRequest *request)
{
  Serial.println("getData");

  // Take Picture with Camera
  CaptureInfo info;
  if (!take_picture(&info)) {
    Serial.println(info.error.c_str());
    request->send(500, "text/plain", info.error.c_str());
    return;
  }

  fs::FS& fs = SD_MMC;
  AsyncWebServerResponse *response = request->beginResponse(fs, info.picturePath, "image/jpeg", true);
  response->addHeader("Access-Control-Allow-Origin", "*"); // Allow requests from any origin (CORS)
  request->send(response);
}



// Arduino main methods

// Initialization
void setup() {
  Serial.begin(115200);

  // WiFi setup
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.print("\nConnected! IP Address: ");
  Serial.println(WiFi.localIP());
  setup_routing();

  // Camera setup
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  // Init SD Card
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  
}


// Main loop
void loop() {
  
}


