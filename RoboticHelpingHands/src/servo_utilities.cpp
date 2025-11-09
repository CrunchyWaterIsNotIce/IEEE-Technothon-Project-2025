#include "servo_utilities.h"

#include <math.h>
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
    _servo.setPeriodHertz(100); // standard 50hz servo
    _servo.attach(pin, 500, 2500); // min-max 500us-2400us for SG90

    delay(15);
    safe_servo_write(angle); // sets servo write to current angle
    delay(15);
    return true;
}

void ServoController::safe_servo_write(int angle) {
    if (!_isAttached) return;
    int constrained = constrain(angle, _boundaries[0], _boundaries[1]);
    
    if (abs(constrained - _currentAngle) >= _tolerance) {
        _currentAngle = constrained;
        _servo.write(_currentAngle);
    }
}

int ServoController::get_current_angle() {
    if (!_isAttached) return -1;
    return _currentAngle;
}

void ServoController::move_to(String type, int to_angle) {
    if (!_isAttached) return;
    
    int start_angle = get_current_angle();
    int delta = to_angle - start_angle;
    if (delta == 0) return; // at target
    
    int steps = 100;
    int delay_ms = 10;
    
    if (type == "cos") {
        for (int i = 0; i <= steps; i++) {
            float t = (float)i / steps;
            float ease = (1.0 - cos(t * PI)) / 2.0;
            int new_angle = start_angle + (int)(delta * ease);
            safe_servo_write(new_angle);
            delay(delay_ms);
        }
    } 
    else if (type == "sin") {
        for (int i = 0; i <= steps; i++) {
            float t = (float)i / steps;
            float ease = sin(t * PI / 2.0);
            int new_angle = start_angle + (int)(delta * ease);
            safe_servo_write(new_angle);
            delay(delay_ms);
        }
    }
    else if (type == "linear") {
        for (int i = 0; i <= steps; i++) {
            float t = (float)i / steps;
            int new_angle = start_angle + (int)(delta * t);
            safe_servo_write(new_angle);
            delay(delay_ms);
        }
    }
    else {
        safe_servo_write(to_angle);
    }
}
