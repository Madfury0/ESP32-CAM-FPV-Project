#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <SD_MMC.h>
#include <ESP32Servo.h>
#include "esp_camera.h"

const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

Servo panServo, tiltServo;

static int photoCount = 0;

void initCamera() {
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
  config.pin_xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init failed");
    return;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  String msg = (char*) data;
  if (msg == "W") { /* Move forward */ }
  else if (msg == "S") { /* Move backward */ }
  else if (msg == "A") { /* Turn left */ }
  else if (msg == "D") { /* Turn right */ }
  else if (msg.startsWith("pan")) { /* Update pan servo based on value */ }
  else if (msg.startsWith("tilt")) { /* Update tilt servo based on value */ }
  else if (msg == "capture") { captureAndSave(); }
  else if (msg == "toggle_light") { /* Toggle light function */ }
}

void captureAndSave() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) return;

  if (!SD_MMC.begin()) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  String filename = "/photos/photo_" + String(photoCount++) + ".jpg";
  
  File file = SD_MMC.open(filename.c_str(), FILE_WRITE);
  if(file) {
    file.write(fb->buf, fb->len);
    file.close();
    Serial.println("Photo saved: " + filename);
  } else {
    Serial.println("Failed to open file for writing");
  }
  
  esp_camera_fb_return(fb);
}

void streamCamera(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginChunkedResponse("multipart/x-mixed-replace; boundary=frame", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return 0;
    size_t len = snprintf((char *)buffer, maxLen, "--frame\r\nContent-Type: image/jpeg\r\n\r\n");
    memcpy(buffer + len, fb->buf, fb->len);
    len += fb->len;
    memcpy(buffer + len, "\r\n", 2);
    len += 2;
    esp_camera_fb_return(fb);
    return len;
  });
  request->send(response);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(1000);

  if(!LITTLEFS.begin()) {
    Serial.println("LittleFS Mount Failed");
  }
  
  if(!SD_MMC.begin()) {
    Serial.println("SD_MMC Mount Failed");
  }
  
  initCamera();

  ws.onEvent(handleWebSocketMessage);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { 
    request->send(LITTLEFS, "/index.html", "text/html"); 
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) { 
    request->send(LITTLEFS, "/style.css", "text/css"); 
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) { 
    request->send(LITTLEFS, "/script.js", "application/javascript"); 
  });
  
  server.on("/stream", HTTP_GET, streamCamera);

  server.begin();
}

void loop() { 
  ws.cleanupClients(); 
}
