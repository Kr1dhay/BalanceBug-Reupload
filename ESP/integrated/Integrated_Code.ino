#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ContinuousStepper.h>
#include <HardwareSerial.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <iostream>

// CAMERA TO ESP VARS
#define BAUD_RATE 115200
#define TX2 17
#define RX2 16
#define SERIAL_MODE SERIAL_8N1

uint32_t data = 0; // data -> full after 4 bytes have been added
uint8_t byteNum = 0; // tracks the number of bytes that have been added to the data variable
uint8_t dataCount = 0; // flag to see if the full data packet is ready to be decoded -> 4 32bit words
bool packetFull = false;
uint32_t dataPacket[4];
uint16_t redDistance;
uint16_t blueDistance;
uint16_t yellowDistance;
uint16_t walls;

// MOTOR VARS
static ContinuousStepper left_stepper, right_stepper;

// WIFI VARS 

#define wifi_ssid = "Vincenzo"
#define password = "hello1234"
#define wifi_ssid_backup = ""
#define password_backup = ""
String URL = "https://balance-bug.5959pn4l16bde.eu-west-2.cs.amazonlightsail.com/";

// #####################################################################################
// START OF WIFI FUNCTIONS

bool connectToWiFi(){
  WiFi.begin(WIFI_SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    // add a failsafe here i.e if it cant connect to one of our phones we try connect to the other
  }
  return true
}

// #####################################################################################
// END OF WIFI FUNCTIONS

// #####################################################################################
// START OF MOTOR FUNCTIONS

void getDirections(){
  if(WiFi.status() == WL_CONNECTED ) {
    WiFiClient client;
    HTTPClient http;

    //recieving rover data
    http.begin(client, URL + "api/getdirections"); // TEMPORARY ENDPOINT
    int httpResponseGet = http.GET(); // dont need to pass arguments
    String payload = http.getString(); // just text that says lft rgt etc (i assume)
    // check the response code and if error then request resend otherwise we cant get the data for the wheels which is VERY IMPORTANT
    Serial.print("HTTP response code: " + String(httpResponseGet) + "\n");
    Serial.print("HTTP payload is: " + payload + "\n");
    command = payload.trim(); // remove any whitespace just in case of accidental error
    http.end();
  }
}

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

void decode(uint32_t frame[4]){
  for(int i = 0; i < 4; i++ ){
    uint32_t identifier = (frame[i] >> 24) & 0xFF; // extracts the most significant 8 bits for the identifier.
    uint32_t distance = frame[i] | 0b11111111111; // bottom 11 bits is used for distance
    
    switch(identifier){
      case 0x79: // red
        redDistance = distance;
        break;
      case 0x62: // blue
        blueDistance = distance;
        break;
      case 0x72: // yellow
        yellowDistance = distance;
        break;
      case 0x0: // lines
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

	// SET UP MOTOR OUTPUTS
	Left_stepper.begin(2, 15);
  	left_stepper.spin(0.1);
  	right_stepper.begin(17, 16);
  	right_stepper.spin(0.1);

}

void loop() {
	while(!connectToWiFi()) {
		// do nothing as we dont have a connection 
		// maybe dont need this if we decide to process image data on esp32
	}

	packetFull = false;
	while(Serial2.available() && !packetFull){
		uint8_t byte = Serial2.read();
		data = data | (uint32_t)byte << (8 * byteNum); 
		byteNum++;
		if(byteNum == 4){
			byteNum = 0;
			data = 0;
			dataCount++;
		}
		if(dataCount == 4){
			packetFull = true;
			decode(dataPacket);
		}
	}

	right_stepper.loop();
	left_stepper.loop();

	// EVENTUALLY REMOVE ALL THE SERIAL PRINT STUFF
	if (command == "fwd") {
		run_motor(-250,+250);
	} else if (command == "bck") {
		run_motor(250,-250);
	} else if (command == "lft") {
		run_motor(-250,250/4);
	} else if (command == "rht") {
		run_motor(-250/4,250);
	} else if (command == "stop") {
		run_motor(0,0);
	} else {
	}

}
