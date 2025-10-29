#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <SparkFun_APDS9960.h>

#define INTERUPT_PIN_APDS 5
SparkFun_APDS9960 gesture_sensor = SparkFun_APDS9960();
volatile bool isr_flag = false;

void handleGesture() {
  if (gesture_sensor.isGestureAvailable()) {
    switch (gesture_sensor.readGesture()) {
      case DIR_UP:
        Serial.println("swiped UP");
        break;
      case DIR_DOWN:
        Serial.println("swiped DOWN");
        break;
      case DIR_LEFT:
        Serial.println("swiped LEFT");
        break;
      case DIR_RIGHT:
        Serial.println("swiped RIGHT");
        break;
      case DIR_FAR:
        Serial.println("swiped FAR");
        break;
      case DIR_NEAR:
        Serial.println("swiped NEAR");
        break;
      default:
        Serial.println("none");
    }
  }
}

void interruptRoutine() {
  isr_flag = true;
}

void setup() {
  // Initializes serial port number for Serial Monitor
  Serial.begin(115200);
  
  attachInterrupt(INTERUPT_PIN_APDS, interruptRoutine, FALLING);
  if (gesture_sensor.init()) Serial.println("apds is initialized");
  else Serial.println("apds failed to initialize");

  if (gesture_sensor.enableGestureSensor(true)) Serial.println("gesture is initialized");
  else Serial.println("gesture failed to initialize");
}

void loop() {
  if (isr_flag) {
    isr_flag = false;
    handleGesture();
  }
  delay(50);
}