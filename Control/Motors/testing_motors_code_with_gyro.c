// Basic demo for accelerometer readings from Adafruit MPU6050

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#include <Kalman.h>

Adafruit_MPU6050 mpu;

const int dirPin_r = 16;
const int stepPin_r = 17;
const int dirPin_l = 15;
const int stepPin_l = 2;

const int stepsPerRevolution = 50;

bool mpu_begin = false;

float accelXAvg = 0.0, accelYAvg = 0.0, accelZAvg = 0.0;
float gyroXAvg = 0.0, gyroYAvg = 0.0, gyroZAvg = 0.0;

// Complementary filter constant
float alpha = 0.96;

float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;

void setup(void) {
  Serial.begin(115200);

  Serial.println("Adafruit MPU6050 test!");

  // Try to initialize!
  while (mpu_begin == false) {
    Serial.println("Not found");
    if (mpu.begin()) {
      mpu_begin = true;
    }
  }
  Serial.println("MPU6050 Found!");

  // motor setup
  pinMode(dirPin_r, OUTPUT);
  pinMode(dirPin_l, OUTPUT);
  pinMode(stepPin_r, OUTPUT);
  pinMode(stepPin_l, OUTPUT);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  calculateError();
}

void loop() {
  sensorFusion();

  Serial.print(roll);
  Serial.print("/");
  Serial.print(pitch);
  Serial.print("/");
  Serial.println(yaw);

  // Serial.print("Gyro ");
  // Serial.print(GyroX);
  // Serial.print("/");
  // Serial.print(GyroX);
  // Serial.print("/");
  // Serial.println(GyroZ);

  delay(100);

  if (pitch < -0.1) {
    // Set motor direction clockwise
    digitalWrite(dirPin_r, LOW);
    digitalWrite(dirPin_l, HIGH);

    for (int x = 0; x < 200; x++) {
      digitalWrite(stepPin_r, HIGH);
      digitalWrite(stepPin_l, HIGH);
      delayMicroseconds(2500);
      digitalWrite(stepPin_r, LOW);
      digitalWrite(stepPin_l, LOW);
      delayMicroseconds(2500);
    }
  } else if (pitch > 0.1) {
    digitalWrite(dirPin_r, HIGH);
    digitalWrite(dirPin_l, LOW);
    for (int x = 0; x < 200; x++) {
      digitalWrite(stepPin_r, HIGH);
      digitalWrite(stepPin_l, HIGH);
      delayMicroseconds(2500);
      digitalWrite(stepPin_r, LOW);
      digitalWrite(stepPin_l, LOW);
      delayMicroseconds(2500);
    }
  }
}

void calculateError() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  int numSamples = 200;
  for (int i = 0; i < numSamples; i++) {
    AccX = a.acceleration.x * 8.0 / 32767;  // X-axis value
    AccY = a.acceleration.y * 8.0 / 32767;  // Y-axis value
    AccZ = a.acceleration.z * 8.0 / 32767;  // Z-axis value

    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
  }
  AccErrorX = AccErrorX / numSamples;
  AccErrorY = AccErrorY / numSamples;

  for (int i = 0; i < numSamples; i++) {
    GyroX = g.gyro.x * 500.0 / 32767;  // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
    GyroY = g.gyro.y * 500.0 / 32767;
    GyroZ = g.gyro.z * 500.0 / 32767;

    GyroErrorX = GyroErrorX + (GyroX * 500.0 / 32767);
    GyroErrorY = GyroErrorY + (GyroY * 500.0 / 32767);
    GyroErrorZ = GyroErrorZ + (GyroZ * 500.0 / 32767);
  }
  GyroErrorX = GyroErrorX / numSamples;
  GyroErrorY = GyroErrorY / numSamples;
  GyroErrorZ = GyroErrorZ / numSamples;

  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);
}

void sensorFusion() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  AccX = a.acceleration.x * 8.0 / 32767;
  AccY = a.acceleration.y * 8.0 / 32767;
  AccZ = a.acceleration.z * 8.0 / 32767;

  // Can hardcode error values
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorX;
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI) - AccErrorY;

  previousTime = currentTime;
  currentTime = millis();
  elapsedTime = (currentTime - previousTime) / 1000;

  GyroX = g.gyro.x * 500.0 / 32767;
  GyroY = g.gyro.y * 500.0 / 32767;
  GyroZ = g.gyro.z * 500.0 / 32767;
  // Can hardcode errors below
  GyroX = GyroX - GyroErrorX;
  GyroY = GyroY - GyroErrorY;
  GyroZ = GyroZ - GyroErrorZ;

  gyroAngleX = gyroAngleX + GyroX * elapsedTime;
  gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  yaw = yaw + GyroZ * elapsedTime;

  // Complementary filter
  roll = alpha * gyroAngleX + (1 - alpha) * accAngleX;
  pitch = alpha * gyroAngleY + (1 - alpha) * accAngleY;
}
