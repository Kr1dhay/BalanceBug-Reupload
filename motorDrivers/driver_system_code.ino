#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ContinuousStepper.h>

static ContinuousStepper left_stepper, right_stepper;
String myname = ""; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  left_stepper.begin(2, 15);
  left_stepper.spin(0.1);
  right_stepper.begin(17, 16);
  right_stepper.spin(0.1);

}

void loop() {
  right_stepper.loop();
  left_stepper.loop();
  if (Serial.available()) {
    String command = Serial.readString();

    command.trim(); // Remove whitespace from the beginning and end of the string
  

    if (command == "fwd") {
      Serial.println("forward");
      run_motor(-250,+250);
    } else if (command == "bck") {
      Serial.println("back");
      run_motor(250,-250);
    } else if (command == "lft") {
      Serial.println("left");
      run_motor(-250,250/4);
    } else if (command == "rht") {
      Serial.println("right");
      run_motor(-250/4,250);
    } else if (command == "stop") {
      Serial.println("stop");
      run_motor(0,0);
    } else {
      Serial.println("invalid");
    }
  }

}

void run_motor(int speed_left, int speed_right) {
  right_stepper.stop();
  left_stepper.stop();
  right_stepper.spin(speed_right);
  left_stepper.spin(speed_left);
}
