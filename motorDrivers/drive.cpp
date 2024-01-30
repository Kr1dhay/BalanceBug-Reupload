#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>



const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.106:1880/update-sensor";

void setup() {
  Serial.begin(115200); 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}


void stop(){
          analogWrite(LPWM,0);
          analogWrite(RPWM,0);
          digitalWrite(LDIR,0);
          digitalWrite(RDIR,0);
}
// This section of code is the main loop
//It constantly checks for UDP signals from the user and when received the rover's movement is controlled or the sensors are activated as required

void loop() 
{
  int packetSize = Udp.parsePacket();
  Serial.println("packet recieved");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Send GET request
    http.begin(serverName);
    int httpResponseCode = http.GET();

    if (httpResponseCode == HTTP_CODE_OK) {
      String payload = http.getString();

      // Parse JSON data
      StaticJsonDocument<200> jsonDocument;
      DeserializationError error = deserializeJson(jsonDocument, payload);

      if (error) {
        Serial.println("Failed to parse JSON");
      } else {
        // Access JSON data
        int value = jsonDocument["value"];

        // Process the JSON data as needed
        if (value == 'fwd'){ //slow forward
          analogWrite(LPWM,50);
          analogWrite(RPWM,50);
          digitalWrite(LDIR,1);
          digitalWrite(RDIR,1);
          sleep(1000);              // This is to only activate the motors for 1s at a time to allow for more precise control
          stop();
        } else if(value == 'bck'){ // backward
          analogWrite(LPWM,50);
          analogWrite(RPWM,50);
          digitalWrite(LDIR,0);
          digitalWrite(RDIR,0);
          sleep(1000);
          stop();
        } else if(value == 'lft'){ // left
          analogWrite(LPWM,20);
          analogWrite(RPWM,20);
          digitalWrite(LDIR,0);
          digitalWrite(RDIR,1);
          sleep(1000);
          stop();
        } else if(value == 'rht'){ // right
          analogWrite(LPWM,20);
          analogWrite(RPWM,20);
          digitalWrite(LDIR,1);
          digitalWrite(RDIR,0);
          sleep(1000);
          stop();
        } else if(value == 'stop'){ // stop
          stop();
        }

        // Udp.beginPacket(Udp.remoteIP(), 55125);
        // Udp.write("RPLY");
      }
    } else {
      Serial.print("HTTP request failed with error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

}