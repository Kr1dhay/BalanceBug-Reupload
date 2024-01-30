#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ContinuousStepper.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <iostream>
#include <Arduino_JSON.h>

// CAMERA TO ESP VARS
#define BAUD_RATE 115200
#define TX2 17
#define RX2 16
#define SERIAL_MODE SERIAL_8N1

TaskHandle_t Task1;
TaskHandle_t Task2;

uint32_t data = 0;      // data -> full after 4 bytes have been added
uint8_t byteNum = 0;    // tracks the number of bytes that have been added to the data variable
uint8_t dataCount = 0;  // flag to see if the full data packet is ready to be decoded -> 4 32bit words
bool packetFull = false;
uint32_t dataPacket[4];
uint16_t redDistance;
uint16_t blueDistance;
uint16_t yellowDistance;
uint16_t walls;

// MOTOR VARS
static ContinuousStepper left_stepper, right_stepper;

// Wi-Fi network credentials
const char* ssid = "Hanbo";
const char* password = "12345678";
const char* serverName = "https://balance-bug.5959pn4l16bde.eu-west-2.cs.amazonlightsail.com/api/rawdata";

// #####################################################################################
// START OF WIFI FUNCTIONS

bool connectToWiFi() {
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    // add a failsafe here i.e if it cant connect to one of our phones we try connect to the other
  }
  Serial.println("Connected to WiFi!");
  return true;
}

// #####################################################################################
// END OF WIFI FUNCTIONS

// #####################################################################################
// START OF MOTOR FUNCTIONS

// void getDirections(){
//   if(WiFi.status() == WL_CONNECTED ) {
//     WiFiClient client;
//     HTTPClient http;

//     //recieving rover data
//     http.begin(client, URL + "api/getdirections"); // TEMPORARY ENDPOINT
//     int httpResponseGet = http.GET(); // dont need to pass arguments
//     String payload = http.getString(); // just text that says lft rgt etc (i assume)
//     // check the response code and if error then request resend otherwise we cant get the data for the wheels which is VERY IMPORTANT
//     Serial.print("HTTP response code: " + String(httpResponseGet) + "\n");
//     Serial.print("HTTP payload is: " + payload + "\n");
//     command = payload.trim(); // remove any whitespace just in case of accidental error
//     http.end();
//   }
// }

void run_motor(int speed_left, int speed_right) {
  right_stepper.stop();
  left_stepper.stop();
  right_stepper.spin(speed_right);
  left_stepper.spin(speed_left);
}

// #####################################################################################
// END OF MOTOR FUNCTIONS

// #####################################################################################
// START OF CAMERA TO ESP FUNCTIONS

void decode(uint32_t frame[4]) {
  for (int i = 0; i < 4; i++) {
    uint32_t identifier = (frame[i] >> 24) & 0xFF;  // extracts the most significant 8 bits for the identifier.
    uint32_t distance = frame[i] | 0b11111111111;   // bottom 11 bits is used for distance

    Serial.println("identifier:"+ identifier);
    Serial.println("distance:"+ identifier);

    switch (identifier) {
      case 0x79:  // red
        redDistance = distance;
        break;
      case 0x62:  // blue
        blueDistance = distance;
        break;
      case 0x72:  // yellow
        yellowDistance = distance;
        break;
      case 0x0:  // lines
        walls = (distance | 0b111);
        break;
    }
  }
}

// #####################################################################################
// END OF CAMERA TO ESP FUNCTIONS


void setup() {

  // SET UP SERIAL CONNECTIONS
  Serial.begin(BAUD_RATE);
  Serial2.begin(BAUD_RATE, SERIAL_MODE, RX2, TX2);

  connectToWiFi();

    //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500);

    //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500);

  // SET UP MOTOR OUTPUTS
  left_stepper.begin(2, 15);
  left_stepper.spin(0.1);
  right_stepper.begin(17, 16);
  right_stepper.spin(0.1);
}

void loop() {
  right_stepper.loop();
  left_stepper.loop();
}

//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    // get the motor data
    if (WiFi.status() == WL_CONNECTED) {
      // putRawData();
      getRawData();
    } else {
      Serial.println("Wi-Fi disconnected. Reconnecting...");
      WiFi.begin(ssid, password);
    }
  } 
}

//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    // get the fpga data
    packetFull = false;
    while (Serial2.available() && !packetFull) {
      uint8_t byte = Serial2.read();
      data = data | (uint32_t)byte << (8 * byteNum);
      byteNum++;
      if (byteNum == 4) {
        byteNum = 0;
        data = 0;
        dataCount++;
      }
      if (dataCount == 4) {
        packetFull = true;
        decode(dataPacket);
      }
    }
  }
}

void putRawData() {
  String payload = "{\"left\":\"true\"}"; // Replace with your desired payload
  String response = httpPUTRequest(serverName, payload);

  Serial.println("Response: " + response);
}

String rawData;

void getRawData() {
  rawData = httpGETRequest(serverName);
  JSONVar myObject = JSON.parse(rawData);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  Serial.print("JSON object = ");
  Serial.println(myObject);

  JSONVar keys = myObject.keys();

  for (int i = 0; i < keys.length(); i++) {
    const char* value = (const char*) myObject[keys[i]];
    if (value == "fwd") {
      run_motor(-250,+250);
    } else if (value == "bck") {
      run_motor(250,-250);
    } else if (value == "lft") {
      run_motor(-250,250/4);
    } else if (value == "rht") {
      run_motor(-250/4,250);
    } else if (value == "stop") {
      run_motor(0,0);
    } else {}
    // Serial.print(keys[i]);
    // Serial.print(" = ");
    // Serial.println(value);
  }

}

String httpGETRequest(const char* serverName) {
  WiFiClientSecure client;
  HTTPClient http;

  client.setInsecure();
  http.begin(client, "https://balance-bug.5959pn4l16bde.eu-west-2.cs.amazonlightsail.com/api/pathing");

  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return payload;
}

String httpPUTRequest(const char* serverName, const String& payload) {
  WiFiClientSecure client;
  HTTPClient http;

  client.setInsecure();
  http.begin(client, serverName);
  	
  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.PUT(payload);

  String responsePayload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    responsePayload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return responsePayload;
}
