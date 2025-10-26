#include <Arduino.h>
#include <ESP32Servo.h>

Servo servo1;
#define SIGNAL_PIN_SERVO1 5

int current_servo1_angle = 0;

// Defined Functions
void safe_servo_write(Servo&, int);
int get_current_servo1_angle();

void setup() {
  // Initializes serial port number for Serial Monitor
  Serial.begin(115200);

  // Set up for servo1
  ESP32PWM::allocateTimer(0); // dedicated timer
  servo1.setPeriodHertz(50); // standard 50hz servo
  servo1.attach(SIGNAL_PIN_SERVO1, 500, 2400); // min-max 500us-2400us for SG90
  
  delay(2000);
  safe_servo_write(servo1, 0);
}

void loop() {
  
  Serial.write("Going to 90 degrees\n");
  // Moves servo1 by increments back to 0 degrees
  for (int val = get_current_servo1_angle(); val < 90; val += 2) {
    safe_servo_write(servo1, val);
    delay(30);
  }
  delay(2000);

  

  Serial.write("Going to back to 0 degrees\n");
  // Moves servo1 by increments back to 0 degrees
  for (int val = get_current_servo1_angle(); val > 0; val -= 2) {
    safe_servo_write(servo1, val);
    delay(30);
  }
  delay(2000);
}

void safe_servo_write(Servo& servo, int angle) {
  /* Helps clamp angles between 0 to 180 */
  angle = constrain(angle, 0, 180);
  servo.write(angle);
  current_servo1_angle = angle;
}

int get_current_servo1_angle() {
  /* Returns current servo1 angle */
  return current_servo1_angle;
}