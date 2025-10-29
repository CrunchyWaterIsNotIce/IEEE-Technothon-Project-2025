#include "servo_utilities.h"

#include <Arduino.h>
#include <ESP32Servo.h>

ServoController::ServoController() :
    // ### SERVO EXIST IN UTILITIES ###
    _signalPin(-1),
    _timerNum(-1),  // unallocated default timer
    _currentAngle(0),
    _isAttached(false)
{}

bool ServoController::attach(int pin, int timer, int angle, bool to_attach) {
    if (_isAttached) return false; //cannot attach

    _signalPin = pin;
    _timerNum = timer;
    _currentAngle = angle;
    _isAttached = to_attach;
    
    if (timer >= 0) ESP32PWM::allocateTimer(timer);
    _servo.setPeriodHertz(50); // standard 50hz servo
    _servo.attach(pin, 500, 2500); // min-max 500us-2400us for SG90

    delay(15);
    safe_servo_write(angle); // sets servo write to current angle
    delay(15);
    return true;
}

void ServoController::safe_servo_write(int angle) {
    // if (!_isAttached) return;

    _currentAngle = constrain(angle, 0, 180); // min-max 0-180 degrees for SG90
    _servo.write(_currentAngle);
}

int ServoController::get_current_angle() {
    // if (!_isAttached) return;
    
    return _currentAngle;
}
