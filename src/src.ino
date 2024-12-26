#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <SD_MMC.h>
#include <ESP32Servo.h>
#include "esp_camera.h"

Servo panServo, tiltServo;

//update websocket to handle connections with python
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  String msg = (char*) data;
  //handle messages
}

void captureandsave() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) return;

  if (!sd_mmc.begin()) {
    serial.println("sd card mount failed");
    return;
  }

  string filename = "/photos/photo_" + string(photocount++) + ".jpg";
  
  file file = sd_mmc.open(filename.c_str(), file_write);
  if(file) {
    file.write(fb->buf, fb->len);
    file.close();
    serial.println("photo saved: " + filename);
  } else {
    serial.println("failed to open file for writing");
  }
  
  esp_camera_fb_return(fb);
}


void setup() {
  Serial.begin(115200);

  if(!LITTLEFS.begin()) {
    Serial.println("LittleFS Mount Failed");
  }
  
  if(!SD_MMC.begin()) {
    Serial.println("SD_MMC Mount Failed");
  }
  
}

void loop() { 
}
