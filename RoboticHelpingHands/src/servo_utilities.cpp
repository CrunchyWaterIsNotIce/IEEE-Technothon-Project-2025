#include "servo_utilities.h"

#include <array>
#include <Arduino.h>
#include <ESP32Servo.h>

ServoController::ServoController() :
    // ### SERVO EXIST IN UTILITIES ###
    _signalPin(-1),
    _timerNum(-1),  // unallocated default timer
    _currentAngle(0),
    _isAttached(false),
    _boundaries{0, 180} // min and max for SG90
{}

bool ServoController::attach(int pin, int timer, bool to_attach, int angle, std::array<int, 2> boundary) {
    if (_isAttached) return false; //cannot attach

    _signalPin = pin;
    _timerNum = timer;
    _isAttached = to_attach;
    _currentAngle = angle;
    _boundaries = boundary;
    
    if (timer >= 0) ESP32PWM::allocateTimer(timer);
    _servo.setPeriodHertz(50); // standard 50hz servo
    _servo.attach(pin, 500, 2500); // min-max 500us-2400us for SG90

    delay(15);
    safe_servo_write(angle); // sets servo write to current angle
    delay(15);
    return true;
}

void ServoController::safe_servo_write(int angle) {
    if (_isAttached) {
        _currentAngle = constrain(angle, _boundaries[0], _boundaries[1]);
        _servo.write(_currentAngle);
    }
}

int ServoController::get_current_angle() {
    if (_isAttached) return _currentAngle;
    return -1;
}
