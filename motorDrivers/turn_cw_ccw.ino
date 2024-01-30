#include <ContinuousStepper.h>

#define LEFT_STEPPER_PIN1 2
#define LEFT_STEPPER_PIN2 15
#define RIGHT_STEPPER_PIN1 17
#define RIGHT_STEPPER_PIN2 16

static ContinuousStepper left_stepper, right_stepper;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    left_stepper.begin(LEFT_STEPPER_PIN1, LEFT_STEPPER_PIN2);
    right_stepper.begin(RIGHT_STEPPER_PIN1, RIGHT_STEPPER_PIN2);
}

void loop()
{
    // Rotate 90 degrees clockwise
    turnClockwise(90);

    // Delay for a moment before turning counterclockwise
    delay(1000);

    // Rotate 90 degrees counterclockwise
    turnCounterclockwise(90);

    // Delay for a moment before repeating the process
    delay(1000);
}

void turnClockwise(float angle)
{
    // Calculate the number of steps required to turn the given angle
    float steps = angle * STEPS_PER_DEGREE;

    // Set the stepper motors to rotate in opposite directions
    left_stepper.spin(-1);
    right_stepper.spin(1);

    // Rotate the motors for the calculated number of steps
    for (int i = 0; i < steps; i++) {
        left_stepper.loop();
        right_stepper.loop();
        delayMicroseconds(1000);  // Adjust as needed for desired speed
    }

    // Stop the motors
    left_stepper.stop();
    right_stepper.stop();
}

void turnCounterclockwise(float angle)
{
    // Calculate the number of steps required to turn the given angle
    float steps = angle * STEPS_PER_DEGREE;

    // Set the stepper motors to rotate in opposite directions
    left_stepper.spin(1);
    right_stepper.spin(-1);

    // Rotate the motors for the calculated number of steps
    for (int i = 0; i < steps; i++) {
        left_stepper.loop();
        right_stepper.loop();
        delayMicroseconds(1000);  // Adjust as needed for desired speed
    }

    // Stop the motors
    left_stepper.stop();
    right_stepper.stop();
}