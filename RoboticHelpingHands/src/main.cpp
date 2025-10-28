#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo_utilities.h"

#define SIGNAL_PIN_SERVO_BASE 27
#define SIGNAL_PIN_SERVO_MIDDLE 26
#define SIGNAL_PIN_SERVO_CROSS 25
ServoController servo_base;
ServoController servo_middle;
ServoController servo_cross;

void setup() {
  // Initializes serial port number for Serial Monitor
  Serial.begin(115200);
 
  servo_base.attach(SIGNAL_PIN_SERVO_BASE, 1, 0, true);
  servo_middle.attach(SIGNAL_PIN_SERVO_MIDDLE, 2, 0, true);
  servo_cross.attach(SIGNAL_PIN_SERVO_CROSS, 3, 0, true);
  delay(4000);
}

void loop() {
  for (int i = servo_base.get_current_angle(); i < 180; i += 2) {
    servo_base.safe_servo_write(i);
    delay(15);
  }
  delay(50);
  for (int i = servo_middle.get_current_angle(); i < 180; i += 2) {
    servo_middle.safe_servo_write(i);
    delay(15);
  }
  delay(50);
  for (int i = servo_cross.get_current_angle(); i < 180; i += 2) {
    servo_cross.safe_servo_write(i);
    delay(15);
  }
  delay(50);
  servo_base.safe_servo_write(0);
  servo_middle.safe_servo_write(0);
  servo_cross.safe_servo_write(0);
  delay(50);
}