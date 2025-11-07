#include <Arduino.h>
#include <Wire.h>

#include <ESP32Servo.h>
#include <SparkFun_APDS9960.h>

#include "servo_utilities.h"

// Gesture Sensor Properties
TwoWire I2C_left = TwoWire(0);
TwoWire I2C_right = TwoWire(1);

SparkFun_APDS9960 left_apds = SparkFun_APDS9960(&I2C_left);
SparkFun_APDS9960 right_apds = SparkFun_APDS9960(&I2C_right);

const int LEFT_SCL_PIN = 22;
const int LEFT_SDA_PIN = 21;

const int RIGHT_SCL_PIN = 17;
const int RIGHT_SDA_PIN = 16;

// Servo Properties
#define SIGNAL_PIN_SERVO_BASE 27
#define SIGNAL_PIN_SERVO_MIDDLE 26
#define SIGNAL_PIN_SERVO_CROSS 25
#define SIGNAL_PIN_SERVO_LEFT 33
#define SIGNAL_PIN_SERVO_RIGHT 32
ServoController servo_base;
ServoController servo_middle;
ServoController servo_cross;
ServoController servo_left;
ServoController servo_right;

// Function Declarations

/* Gesture Sensors */
void apdsInitialization();
void handleGesture(int gesture, const char* sensor_name);
int readGestureNonBlocking(SparkFun_APDS9960& apds);

/* Servos */
void servoInitialization();

void setup() {
  // Initializes serial port number for Serial Monitor
  Serial.begin(115200);
 
  apdsInitialization();
  // servoInitialization();

  delay(4000);
}

void loop() {
  // servo_base.safe_servo_write(0);
  // servo_middle.safe_servo_write(0);
  // servo_cross.safe_servo_write(0);
  // servo_left.safe_servo_write(0);
  // servo_right.safe_servo_write(0);

  int left_gesture = DIR_NONE;
  int right_gesture = DIR_NONE;
  
  if (left_apds.isGestureAvailable()) {
    left_gesture = readGestureNonBlocking(left_apds);
  }
  
  if (right_apds.isGestureAvailable()) {
    right_gesture = readGestureNonBlocking(right_apds);
  }
  
  // Handle gestures from both sensors
  if (left_gesture != DIR_NONE && left_gesture != -1) {
    handleGesture(left_gesture, "LEFT");
  }
  
  if (right_gesture != DIR_NONE && right_gesture != -1) {
    handleGesture(right_gesture, "RIGHT");
  }
  
  // If both sensors detected gestures simultaneously
  if (left_gesture != DIR_NONE && left_gesture != -1 && 
      right_gesture != DIR_NONE && right_gesture != -1) {
    Serial.println(">>> SIMULTANEOUS GESTURES DETECTED <<<");
  }

  delay(10);
}

void apdsInitialization() {
  I2C_left.begin(LEFT_SDA_PIN, LEFT_SCL_PIN, 100000);
  I2C_right.begin(RIGHT_SDA_PIN, RIGHT_SCL_PIN, 100000);

  if (left_apds.init()) Serial.println("Left sensor initialized");
  else Serial.println("Left sensor failed");

  if (right_apds.init()) Serial.println("Right sensor initialized");
  else Serial.println("Right sensor failed");

  if (left_apds.enableGestureSensor(true)) Serial.println("Left gesture enabled");
  else Serial.println("Left gesture failed");

  if (right_apds.enableGestureSensor(true)) Serial.println("Right gesture enabled");
  else Serial.println("Right gesture failed");
}

void handleGesture(int gesture, const char* sensor_name) {
  Serial.print(sensor_name);
  Serial.print(": ");
  
  switch (gesture) {
    case DIR_UP:
      Serial.println("UP");
      break;
    case DIR_DOWN:
      Serial.println("DOWN");
      break;
    case DIR_LEFT:
      Serial.println("LEFT");
      break;
    case DIR_RIGHT:
      Serial.println("RIGHT");
      break;
    case DIR_FAR:
      Serial.println("FAR");
      break;
    case DIR_NEAR:
      Serial.println("NEAR");
      break;
    default:
      Serial.println("unknown");
  }
}

int readGestureNonBlocking(SparkFun_APDS9960& apds) {
  unsigned long start = millis();
  const unsigned long timeout = 100;
  
  while (millis() - start < timeout) {
    if (apds.isGestureAvailable()) {
      int gesture = apds.readGesture();
      if (gesture != -1) {
        return gesture;
      }
    }
    delay(5);
  }
  
  return DIR_NONE;
}

void servoInitialization() {
  servo_base.attach(SIGNAL_PIN_SERVO_BASE, 1, true, 0, {0, 90});
  servo_middle.attach(SIGNAL_PIN_SERVO_MIDDLE, 2, true, 0, {0, 90});
  servo_cross.attach(SIGNAL_PIN_SERVO_CROSS, 3, true, 0, {0, 180});
  servo_left.attach(SIGNAL_PIN_SERVO_LEFT, 4, true, 0, {0, 180});
  servo_right.attach(SIGNAL_PIN_SERVO_RIGHT, 5, true, 0, {0, 180});
}