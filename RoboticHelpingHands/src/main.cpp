#include <Arduino.h>
#include <ESP32Servo.h>

Servo servo1;
#define SIGNAL_PIN_SERVO1 5

void safeServoWrite(Servo &servo, int angle) {
  angle = constrain(angle, 0, 180);  // clamping
  servo.write(angle);
}

void setup() {
  Serial.begin(115200);
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  ESP32PWM::allocateTimer(4);

  servo1.setPeriodHertz(50); // standard 50hz servo
  servo1.attach(SIGNAL_PIN_SERVO1, 500, 2400); // min-max 500us-2400us for SG90
  delay(2000);
  servo1.write(0);
}

void loop() {
  Serial.write("Going to 180 degrees\n");
  for (int val = 0; val < 180; val += 2) {
    safeServoWrite(servo1, val);
    delay(30);
  }
  delay(2000);

  Serial.write("Going to back to 0 degrees\n");
  for (int val = 180; val > 0; val -= 2) {
    safeServoWrite(servo1, val);
    delay(30);
  }
  delay(2000);
}