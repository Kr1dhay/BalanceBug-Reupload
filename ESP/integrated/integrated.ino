#include <HardwareSerial.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "freertos/task.h"
#include <ContinuousStepper.h>

#define WIFI_SSID     "jamies_iphone"
#define PASSWORD      "jamieturner2"
String URL          = "https://balance-bug.5959pn4l16bde.eu-west-2.cs.amazonlightsail.com/";
String LOCAL_HOST   = "localhost:3000/";
String AWS_ENDPOINT = "/api/update"; // exposed port

// Random variables for testing POST
bool junction = false;
unsigned int xpos = 100;
unsigned int ypos = 10;
unsigned int orientation = 24;
unsigned int line_array[6] = {0, 1, 0, 0, 1, 0};

TaskHandle_t uartMotor;
TaskHandle_t espServer;

static ContinuousStepper left_stepper, right_stepper;

// #################################################################################################################################################
// #################################################################################################################################################

// Establish WiFi connection
void connectToWiFi(){
  WiFi.begin(WIFI_SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    Serial.println(".");
    delay(1000);
  }
  Serial.println("Connected to internet!");
}

void httpRequests(){
  if(WiFi.status() == WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;

    // sending data to the rover
    String update_path = URL + "/api/update"; // need to include endpoint in again
    http.begin(client, update_path); // could need a request number at the end, not too sure
    int httpResponsePost = http.POST("{\"xpos\": " + String(xpos) + ",\n\"ypos\": " + String(ypos) + ",\n\"degrees\": " + String(orientation) + "'\n\"lines\": [" + String(line_array[0]) + ", " + String(line_array[1]) + ", " + String(line_array[2]) + ", " + String(line_array[3]) + ", " + String(line_array[4]) + ", " + String(line_array[5]) + "]");
    Serial.print("HTTP response code: " + String(httpResponsePost) + "\n");
    http.end();

    //recieving rover data
    http.begin(client, URL + "api/recieve");
    int httpResponseGet = http.GET(); // dont need to pass arguments
    String payload = http.getString(); // no clue what the format of payload is... it includes values that need to be passed to the drive control 
    Serial.print("HTTP response code: " + String(httpResponseGet) + "\n");
    Serial.print("HTTP payload is: " + payload + "\n");
    http.end();

  }else{
    Serial.println("connection lost");
  }
}
// #################################################################################################################################################
// #################################################################################################################################################

// #####################################################################################
// START OF MOTOR FUNCTIONS
void setup() {
  connectToWiFi();

  xTaskCreatePinnedToCore(
    uart_motor,
    "UART-and-Motor",
    10000,
    NULL,
    1,
    &uartMotor,
    0
  );
  xTaskCreatePinnedToCore(
    esp_server,
    "ESP-and-Server",
    10000,
    NULL,
    1,
    &espServer,
    1
  );

  // SET UP MOTOR OUTPUTS - for the time being here although most likely will need to go into the task functions.
	  left_stepper.begin(2, 15);
  	left_stepper.spin(0.1);
  	right_stepper.begin(17, 16);
  	right_stepper.spin(0.1);
}
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
    payload.trim(); // remove any whitespace just in case of accidental error
    http.end();
    if (payload == "fwd") {
		run_motor(-250,+250);
	} else if (payload == "bck") {
		run_motor(250,-250);
	} else if (payload == "lft") {
		run_motor(-250,250/4);
	} else if (payload == "rht") {
		run_motor(-250/4,250);
	} else if (payload == "stop") {
		run_motor(0,0);
	} else {
	}
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





#define BAUD_RATE 115200
#define TX2 17
#define RX2 16
#define SERIAL_MODE SERIAL_8N1
uint32_t data = 0;
uint8_t byteNum = 0;

void uart_motor(void * pvParameters){

  Serial.begin(BAUD_RATE);
  Serial2.begin(BAUD_RATE, SERIAL_MODE, RX2, TX2);
  
  for(;;){
    while(Serial2.available()){
      uint8_t byte = Serial2.read();
      data = data | (uint32_t)byte << (8 * byteNum);    
    }
    delay(1);

    right_stepper.loop();
	  left_stepper.loop();

	// EVENTUALLY REMOVE ALL THE SERIAL PRINT STUFF
	
}
}

  void esp_server(void * pvParameters){
    connectToWiFi();
    for(;;){
      httpRequests();
    }
  }
void loop() {
}
