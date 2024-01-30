// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

Adafruit_MPU6050 mpu;

float x_angle_acc;
float y_angle_acc;
float z_angle_acc;
float acc_angle;

// Use the following global variables and access functions to help store the overall
// rotation angle of the sensor
float         last_x_angle;  // These are the filtered angles
float         last_y_angle;
float         last_z_angle;  
float         last_gyro_x_angle;  // Store the gyro angles to compare drift
float         last_gyro_y_angle;
float         last_gyro_z_angle;

unsigned long previous_time = 0;
float dt = 0.0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("");
  delay(100);
}

void loop() {

  unsigned long current_time = millis();
  dt = (current_time-previous_time) / 1000.0;
  previous_time = current_time;

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  x_angle_acc = atan2(a.acceleration.x, sqrt(pow(a.acceleration.y,2) + pow(a.acceleration.z,2))) * (180/M_PI);
  y_angle_acc = atan2(a.acceleration.y, sqrt(pow(a.acceleration.x,2) + pow(a.acceleration.z,2))) * (180/M_PI); 
  z_angle_acc = 0;
  //acc_angle   = (x_angle_acc/y_angle_acc) * (180/M_PI);

  float gyro_angle_x = g.gyro.x / 65.5;
  float gyro_angle_y = g.gyro.y / 65.5;
  float gyro_angle_z = g.gyro.z / 65.5;
  
  gyro_angle_x = gyro_angle_x*dt + last_x_angle;
  gyro_angle_y = gyro_angle_y*dt + last_y_angle;
  gyro_angle_z = gyro_angle_z*dt + last_z_angle;

  float alpha = 0.98;
  float angle_x = alpha*gyro_angle_x + (1.0 - alpha)*x_angle_acc;
  float angle_y = alpha*gyro_angle_y + (1.0 - alpha)*y_angle_acc;
  float angle_z = gyro_angle_z;  //Accelerometer doesn't give z-angle
  
  // Output the angles to the serial plotter
  Serial.print("x:");
  Serial.print(angle_x);
  Serial.print('\t');
  Serial.print("y:");
  Serial.print(angle_y);
  Serial.print('\t');
  Serial.print("z:");
  Serial.println(angle_z);

  last_x_angle = angle_x;
  last_y_angle = angle_y;
  last_z_angle = angle_z; 

  delay(10);
}