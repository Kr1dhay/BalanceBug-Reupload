#include <Wire.h>
#include <MPU6050.h>
#include <imuFilter.h>
#include <AutoPID.h>


MPU6050 accelgyro;

#define runEvery(t) for (static long _lasttime;\
                         (uint16_t)((uint16_t)millis() - _lasttime) >= (t);\
                         _lasttime += (t))


int16_t ax, ay, az;
float accBiasX, accBiasY, accBiasZ;
float accAngleX, accAngleY;
double accPitch, accRoll;

int16_t gx, gy, gz;
float gyroBiasX, gyroBiasY, gyroBiasZ;
float gyroRateX, gyroRateY, gyroRateZ;
float gyroBias_oldX, gyroBias_oldY, gyroBias_oldZ;
float gyroPitch = 180;
float gyroRoll = -180;
float gyroYaw = 0;

uint32_t timer;

// input
double InputPitch, InputRoll;

// initial values
double InitialRoll;

// Motors
int a1 = 5;
int a2 = 4;
int b1 = 10;
int b2 = 11;

void setup() {

  Wire.begin();

  Serial.begin(115200);

  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  delay(1500);

  // Motor
  pinMode(a1, OUTPUT);
  pinMode(a2, OUTPUT);
  pinMode(b1, OUTPUT);
  pinMode(b2, OUTPUT);

  digitalWrite(a1, HIGH);
  digitalWrite(a2, HIGH);
  digitalWrite(b1, HIGH);
  digitalWrite(b2, HIGH);
  
 //TODO: Better calibration 
  accelgyro.setXAccelOffset(-250);
  accelgyro.setYAccelOffset(-1929);
  accelgyro.setZAccelOffset(1077);
  accelgyro.setXGyroOffset(98);
  accelgyro.setYGyroOffset(-145);
  accelgyro.setZGyroOffset(-22);

  gyroBiasX = 0;
  gyroBiasY = 0;
  gyroBiasZ = 0;

  accBiasX = 4;
  accBiasY = -4;
  accBiasZ = 16378;

  //Get Starting Pitch and Roll
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  accPitch = (atan2(-ax, -az) + PI) * RAD_TO_DEG;
  accRoll = (atan2(ay, -az) + PI) * RAD_TO_DEG;

  if (accPitch <= 360 & accPitch >= 180) {
    accPitch = accPitch - 360;
  }

  if (accRoll <= 360 & accRoll >= 180) {
    accRoll = accRoll - 360;
  }

  gyroPitch = accPitch;
  gyroRoll = accRoll;

  timer = micros();
  delay(1000);
  initializeValues();

}

double Setpoint;

void MotorControl(double out) {
  if (out > 0) {
    digitalWrite(a1, LOW);
    digitalWrite(a2, HIGH);
    digitalWrite(b1, HIGH);
    digitalWrite(b2, LOW);
  } else {
    digitalWrite(a1, HIGH);
    digitalWrite(a2, LOW);
    digitalWrite(b1, LOW);
    digitalWrite(b2, HIGH);
  }

  byte vel = abs(out);
  if (vel < 0)
    vel = 0;
  if (vel > 255)
    vel = 255;
}

void initializeValues() {

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  //////////////////////
  //  Accelerometer   //
  //////////////////////
  accPitch = (atan2(-ax/182.0, -az/182.0) + PI) * RAD_TO_DEG;
  accRoll = (atan2(ay/182.0, -az/182.0) + PI) * RAD_TO_DEG;

  if (accRoll <= 360 & accRoll >= 180) {
    accRoll = accRoll - 360;
  }

  //////////////////////
  //      GYRO        //
  //////////////////////

  gyroRateX = ((int)gx - gyroBiasX) * 131; 

  gyroPitch += gyroRateY * ((double)(micros() - timer) / 1000000);

  
  timer = micros();
  InitialRoll = accRoll;

  Setpoint = InitialRoll;
}

double filtered = 0;
void loop() {



  /* Get new sensor events with the readings */
  //sensors_event_t a, g, temp;
  //mpu.getEvent(&a, &g, &temp);
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Print accelerometer and gyroscope readings
  Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(ax);
  Serial.print(", Y = "); Serial.print(ay);
  Serial.print(", Z = "); Serial.print(az);

  Serial.print(" | ");

  Serial.print("Gyroscope: ");
  Serial.print("X = "); Serial.print(gx);
  Serial.print(", Y = "); Serial.print(gy);
  Serial.print(", Z = "); Serial.println(gz);

  delay(100);  // Delay between readings

  runEvery(10) {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    //////////////////////
    //  Accelerometer   //
    //////////////////////

    accRoll = (atan2(ay/182.0, -az/182.0) + PI) * RAD_TO_DEG;

    if (accRoll <= 360 & accRoll >= 180) {
      accRoll = accRoll - 360;
    }


    //////////////////////
    //      GYRO        //
    //////////////////////

    gyroRateX = -((int)gx - gyroBiasX) / 131; 


    double gyroVal = gyroRateX * ((double)(micros() - timer) / 1000000);

    timer = micros();

    //Complementary filter
    filtered = 0.98 * (filtered + gyroVal) + 0.02 * (accRoll);

    MotorControl(Compute(filtered - InitialRoll));

  }

}

int outMax = 255;
int outMin = -255;
float lastInput = 0;
double ITerm = 0;
double kp = 100;
double ki = 10;
double kd = 2;

double Compute(double input)
{

  double error = Setpoint - input;

  ITerm += (ki * error);

  if (ITerm > outMax) ITerm = outMax;
  else if (ITerm < outMin) ITerm = outMin;
  double dInput = (input - lastInput);


  /*Compute PID Output*/
  double output = kp * error + ITerm + kd * dInput;

  if (output > outMax) output = outMax;
  else if (output < outMin) output = outMin;

  /*Remember some variables for next time*/
  lastInput = input;
  return output;
}

