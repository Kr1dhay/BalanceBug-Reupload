/*
 * Based upon code written by Yue Zhu (yue.zhu18@imperial.ac.uk) in July 2020.
 * pin6 is PWM output at 62.5kHz.
 * duty-cycle saturation is set as 2% - 98%
 * Control frequency is set as 1kHz. 
*/
#include <Wire.h>
#include <INA219_WE.h>
INA219_WE ina219; // this is the instantiation of the library for the current sensor
float power = 0.0;
float power_prev = 0.0;
float voltage_prev = 0.0;
bool lessthan = false;
float dutycycle = 0.33;
float open_loop, closed_loop; // Duty Cycles
float va,vb,vref,iL,dutyref,current_mA; // Measurement Variables
unsigned int sensorValue0,sensorValue1,sensorValue2,sensorValue3;  
boolean Boost_mode = 0;
boolean CL_mode = 0;
unsigned int loopTrigger;
unsigned int com_count=0;   // a variables to count the interrupts. Used for program debugging.

void setup() {
  //Basic pin setups
  
  noInterrupts(); //disable all interrupts
  pinMode(13, OUTPUT);  //Pin13 is used to time the loops of the controller
  pinMode(3, INPUT_PULLUP); //Pin3 is the input from the Buck/Boost switch
  pinMode(2, INPUT_PULLUP); // Pin 2 is the input from the CL/OL switch
  analogReference(EXTERNAL); // We are using an external analogue reference for the ADC
  // TimerA0 initialization for control-loop interrupt.
  
  TCA0.SINGLE.PER = 999; //
  TCA0.SINGLE.CMP1 = 999; //
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc | TCA_SINGLE_ENABLE_bm; //16 prescaler, 1M.
  TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP1_bm; 
   
  pinMode(6, OUTPUT);
  TCB0.CTRLA=TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm; //62.5kHz
  analogWrite(6,120); 
  Serial.begin(115200);   //serial communication enable. Used for program debugging.
  interrupts();  //enable interrupts.
  Wire.begin(); // We need this for the i2c comms for the current sensor
  ina219.init(); // this initiates the current sensor
  Wire.setClock(700000); // set the comms speed for i2c
  
}
 void loop() {
  if(loopTrigger) { 
    
    digitalWrite(13, HIGH); 
    
    // Sample all of the measurements and check which control mode we are in
    sampling();// Open Loop Boost
    
    //open_loop=saturation(open_loop,0.99,dutyref);
    open_loop = dutycycle;
    pwm_modulate(open_loop); 

    com_count++;              //used for debugging.
    if (com_count >= 500) {  //send out data every second.
      Serial.print("Va: ");
      Serial.print(va);
      Serial.print("\t");
      Serial.print("Vb: ");
      Serial.print(vb);
      Serial.print("\t");
      Serial.print("Inductor Current: ");
      Serial.print(iL);
      Serial.print("\t\t");
      Serial.print("dutyref: ");
      Serial.print(open_loop);
      Serial.print("\t\t");
      Serial.print("Power: ");
      Serial.print(power);
      Serial.print("\t\t");
      Serial.print("\n");
      Serial.print("duty cycle: ");
      Serial.print(dutycycle);
      Serial.print("\n");
      com_count = 0;
    }
    digitalWrite(13, LOW);   // reset pin13.
    loopTrigger = 0;
  }
}

// Timer A CMP1 interrupt. Every 800us the program enters this interrupt. 
// This, clears the incoming interrupt flag and triggers the main loop.
ISR(TCA0_CMP1_vect){
  TCA0.SINGLE.INTFLAGS |= TCA_SINGLE_CMP1_bm; //clear interrupt flag
  loopTrigger = 1;
}
// This subroutine processes all of the analogue samples, creating the required values for the main loop
void sampling(){
  // Make the initial sampling operations for the circuit measurements
  
  sensorValue0 = analogRead(A0); //sample Vb
  sensorValue2 = analogRead(A2); //sample Vref
  sensorValue3 = analogRead(A3); //sample Va
  current_mA = ina219.getCurrent_mA(); // sample the inductor current (via the sensor chip)
  // Process the values so they are a bit more usable/readable
  // The analogRead process gives a value between 0 and 1023 
  // representing a voltage between 0 and the analogue reference which is 4.096V
  
  vb = sensorValue0 * (12400/2400) * (4.096 / 1023.0); 
  vref = sensorValue2 * (4.096 / 1023.0); 
  va = sensorValue3 * (12400/2400) * (4.096 / 1023.0); 
  iL = -current_mA/1000.0;
    
  // THIS IS WHERE THE MPPT ALGORITHM IS IMPLEMENTED
  power = iL * vb;
  // A SLIGHT DELAY IS ADDED
  if (com_count % 20 == 0) {
    if (power > power_prev) {
    // here we need to increase it more towards limit
        if (power > 0 || power_prev > 0) {
          dutycycle = dutycycle + 0.01;
        }
    } else if (power < power_prev) {
      // here we need to move it backwards
      if (power > 0 || power_prev > 0) {
        dutycycle = dutycycle - 0.01;
        }
    } else {
      dutycycle = dutycycle;
    }
    // ensure that the duty cycle doesnt pass the limits 
    if (dutycycle > 0.99) {
      dutycycle = 0.99; // upper limit of the duty cycle
    } else if (dutycycle < 0.33) {
      dutycycle = 0.33; // lower limit of the duty cycle maybe we can change this value 
    }
    dutyref = dutycycle;
    power_prev = power;
  } 
}

float saturation( float sat_input, float uplim, float lowlim){ // Saturatio function
  if (sat_input > uplim) sat_input=uplim;
  else if (sat_input < lowlim ) sat_input=lowlim;
  else;
  return sat_input;
}

void pwm_modulate(float pwm_input){ // PWM function
  analogWrite(6,(int)(255-pwm_input*255)); 
}

/*end of the program.*/
