  #include <esp32-hal-timer.h>
  #include <Adafruit_MPU6050.h>
  #include <Adafruit_Sensor.h>
  #include <Wire.h>
  #include <math.h>
  #include <ContinuousStepper.h>

  // instantiate the steppers
  static ContinuousStepper left_stepper, right_stepper;
  // AccelStepper stepper1(AccelStepper::DRIVER, 2, 15); 
  // AccelStepper stepper2(AccelStepper::DRIVER, 16, 17);

  // instantiate the mpu
  Adafruit_MPU6050 mpu;

  // timer
  hw_timer_t *timer = NULL;
  const int testpin = 19;

  void IRAM_ATTR interruptHandler() {
    // add the code to control the motors here
    digitalWrite(testpin, !digitalRead(testpin));
    left_stepper.loop();
    right_stepper.loop();
  }

  // PID Vars
  float KP = 5.0;
  float KI = 0.0;
  float KD = 0.0;

  float previous_error_x = 0.0;
  float previous_error_y = 0.0;

  float desired_angle_x = 0.0;
  float desired_angle_y = 0.0;

  float integral_term_y = 0.0;
  float integral_term_x = 0.0;

  const int dirPin_r = 16; 
  const int stepPin_r = 17;
  const int dirPin_l = 15;
  const int stepPin_l = 2;

  const int stepsPerRevolution = 50;
  const int stepDelay = 2500;

  float x_angle_acc;
  float y_angle_acc;
  float z_angle_acc;
  float acc_angle;

  // Use the following global variables and access functions to help store the overall
  // rotation angle of the sensor
  float  last_x_angle;  // These are the filtered angles
  float  last_y_angle;
  float  last_z_angle;  
  float  last_gyro_x_angle;  // Store the gyro angles to compare drift
  float  last_gyro_y_angle;
  float  last_gyro_z_angle;

  float pid_output_x;
  float pid_output_y;

  unsigned long previous_time = 0;
  float dt = 0.0;

  float a_errorx = 0.0;
  float a_errory = 0.0;
  float a_errorz = 0.0;

  float g_errorx = 0.0;
  float g_errory = 0.0;
  float g_errorz = 0.0;

  float base_x_accel = 0.0;
  float base_y_accel = 0.0;
  float base_z_accel = 0.0;
  float base_x_gyro = 0.0;
  float base_y_gyro = 0.0;
  float base_z_gyro = 0.0;

  void calibrate_sensors() {
    Serial.println("Calibrating...");
    int    num_readings = 2000;
    float  x_accel = 0;
    float  y_accel = 0;
    float  z_accel = 0;
    float  x_gyro = 0;
    float  y_gyro = 0;
    float  z_gyro = 0;

    sensors_event_t ea, eg, etemp;
    mpu.getEvent(&ea, &eg, &etemp);

    for (int i = 0; i < num_readings; i++) {
      mpu.getEvent(&ea, &eg, &etemp);
      x_accel += ea.acceleration.x / 16384;
      y_accel += ea.acceleration.y / 16384;
      z_accel += ea.acceleration.z / 16384;
      x_gyro += eg.gyro.x / 131;
      y_gyro += eg.gyro.y / 131;
      z_gyro += eg.gyro.z / 131;
      delay(10);
    }
    x_accel /= num_readings;
    y_accel /= num_readings;
    z_accel /= num_readings;
    x_gyro /= num_readings;
    y_gyro /= num_readings;
    z_gyro /= num_readings;

    base_x_accel = x_accel;
    base_y_accel = y_accel;
    base_z_accel = z_accel;
    base_x_gyro = x_gyro;
    base_y_gyro = y_gyro;
    base_z_gyro = z_gyro;

  }

  void setup(void) {
    Serial.begin(115200);
    while (!Serial)
      delay(10); // will pause Zero, Leonardo, etc until serial console opens

    Serial.println("Name Pending Balance Bug Test!");

    // Try to initialize!
    if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      while (1) {
        delay(10);
      }
    }

    pinMode(dirPin_r, OUTPUT);
    pinMode(dirPin_l, OUTPUT);
    pinMode(stepPin_r, OUTPUT);
    pinMode(stepPin_l, OUTPUT);
    pinMode(testpin,OUTPUT);

    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);

    mpu.setGyroRange(MPU6050_RANGE_250_DEG);

    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    calibrate_sensors();
    Serial.println("Calibration Completed");
    Serial.println("");
    delay(20);

    left_stepper.begin(2, 15);
    left_stepper.setAcceleration(600);
    right_stepper.begin(17, 16);
    right_stepper.setAcceleration(600);

    // set up the timer
    timer = timerBegin(0, 20, true);  // Timer 0, prescaler 80, counting up
    timerAttachInterrupt(timer, &interruptHandler, true);  // Attach the interrupt handler
    timerAlarmWrite(timer, 50, true);  // Set the alarm value to 50,000 microseconds (50 milliseconds)
    timerAlarmEnable(timer);
  }

  void loop() {
    // check the control 
    unsigned long current_time = millis();
    dt = (current_time-previous_time) / 1000.0;
    previous_time = current_time;

    // /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float accX = a.acceleration.x / 16384;
    float accY = a.acceleration.y / 16384;
    float accZ = a.acceleration.z / 16384;

    x_angle_acc = (atan2(accY, sqrt(pow(accX,2) + pow(accZ,2))) * 180 / PI) - base_x_accel;   // roll
    y_angle_acc = (atan2(-1*accX, sqrt(pow(accY,2) + pow(accZ,2))) * 180 / PI) - base_y_accel; // pitch

    float gyro_angle_x = (g.gyro.x - base_x_gyro) / 131;
    float gyro_angle_y = (g.gyro.y - base_y_gyro) / 131;
    float gyro_angle_z = (g.gyro.z - base_z_gyro) / 131;
    
    gyro_angle_x = gyro_angle_x*dt + last_x_angle;
    gyro_angle_y = gyro_angle_y*dt + last_y_angle;
    gyro_angle_z = gyro_angle_z*dt + last_z_angle;

    float alpha = 0.97;
    float angle_x = alpha*gyro_angle_x + (1.0 - alpha)*x_angle_acc;
    float angle_y = alpha*gyro_angle_y + (1.0 - alpha)*y_angle_acc;
    float angle_z = gyro_angle_z;  //Accelerometer doesn't give z-angle

    last_x_angle = angle_x;
    last_y_angle = angle_y;
    last_z_angle = angle_z; 

    // calc errors and do PID control 
    float error_x = desired_angle_x - angle_x;
    float error_y = desired_angle_y - angle_y;

    float p_term_y = KP * error_y;
    integral_term_y += KI * error_y * dt;
    float d_term_y = KD * (error_y - previous_error_y) / dt;

    float p_term_x = KP * error_x;
    integral_term_x += KI * error_x * dt;
    float d_term_x = KD * (error_x - previous_error_x) / dt;

    pid_output_x = p_term_x + integral_term_x + d_term_x;
    pid_output_y = p_term_y + integral_term_y + d_term_y;

    Serial.print("PID Y OUTPUT: ");
    Serial.println(pid_output_y);

    left_stepper.spin(pid_output_y * abs(pid_output_y));
    right_stepper.spin(-pid_output_y * abs(pid_output_y));

    delay(10);
  }
