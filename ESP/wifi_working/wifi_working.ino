#include <WiFi.h>

#define WIFI_SSID "jamies_iphone"
#define WIFI_PASSWORD "jamieturner2"
#define WIFI_TIMEOUT_MS 20000

void connectWifi(){
  Serial.print("Connecting to wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while(WiFi.status() != WL_CONNECTED){
    Serial.println(".");
    delay(1000);
  }

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Failed to connect to Jamie's iPhone");
    // stuff like reconnect
  }else{
    Serial.println("Connected to Jamie's iPhone");
  }
}

void setup() {
  Serial.begin(115200);
  connectWifi();
}

void loop() {}
