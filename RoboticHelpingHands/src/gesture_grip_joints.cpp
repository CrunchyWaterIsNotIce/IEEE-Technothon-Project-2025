#include "gesture_grip_joints.h"

GestureGripJoints::GestureGripJoints() :
    _ledState(false),
    _lastBlink(0)
{
    _servoRefs[0] = &_servo_base;
    _servoRefs[1] = &_servo_middle;
    _servoRefs[2] = &_servo_cross;
    _servoRefs[3] = &_servo_left;
    _servoRefs[4] = &_servo_right;
}

bool GestureGripJoints::initialize() {
    initializeLED(); // initialze the leds

    bool success = true;
    success &= _servo_base.attach(_PIN_BASE, 1, true, 0, {20, 80});
    success &= _servo_middle.attach(_PIN_MIDDLE, 2, true, 0, {0, 80});
    success &= _servo_cross.attach(_PIN_CROSS, 3, true, 0, {0, 180});
    success &= _servo_left.attach(_PIN_LEFT, 4, true, 0, {0, 170});
    success &= _servo_right.attach(_PIN_RIGHT, 5, true, 0, {0, 170});

    return success;
}
void GestureGripJoints::moveToUpright() {
    _servo_base.move_to("sin", 20);
    _servo_middle.move_to("sin", 0);
    _servo_cross.move_to("sin", 0);
    _servo_left.move_to("sin", 0);
    _servo_right.move_to("sin", 0);
}

void GestureGripJoints::moveToDownward() {
    _servo_base.move_to("cos", 80);
    _servo_middle.move_to("cos", 80);
    _servo_cross.move_to("cos", 180);
    _servo_left.move_to("cos", 170);
    _servo_right.move_to("cos", 170);
}

void GestureGripJoints::adjustServo(int servo_index, int increment) {
    if (servo_index < 0 || servo_index >= _SERVO_COUNT) return;
    
    ServoController* servo = _servoRefs[servo_index];
    int current = servo->get_current_angle();
    servo->safe_servo_write(current + increment);
    
    Serial.printf("%s: %d° -> %d°\n",
                  _servoLabels[servo_index],
                  current,
                  servo->get_current_angle()
                );
}

int GestureGripJoints::getServoAngle(int servo_index) {
    if (servo_index < 0 || servo_index >= _SERVO_COUNT) return -1;
    return _servoRefs[servo_index]->get_current_angle();
}

const char* GestureGripJoints::getServoLabel(int servo_index) {
    if (servo_index < 0 || servo_index >= _SERVO_COUNT) return "INVALID";
    return _servoLabels[servo_index];
}

void GestureGripJoints::updateLED(int state, int selected_servo) {
    unsigned long now = millis();
    
    switch (state) {
        case 0: // STATE_DIRECT
            setRGBColor(_COLOR_WHITE);
            break;
            
        case 1: // STATE_SELECT_SERVO
            if (now - _lastBlink >= _BLINK_INTERVAL) { // blink selected servo color
                _ledState = !_ledState;
                _lastBlink = now;
            }
            
            if (_ledState && selected_servo >= 0 && selected_servo < _SERVO_COUNT) {
                setRGBColor(_servoColors[selected_servo]);
            } else {
                setRGBColorPWM(0, 0, 0); // Off
            }
            break;
            
        case 2: // STATE_ADJUST_SERVO
            if (selected_servo >= 0 && selected_servo < _SERVO_COUNT) {
                setRGBColor(_servoColors[selected_servo]);
            }
            break;
    }
}

void GestureGripJoints::initializeLED() {
    ledcSetup(6, 5000, 8); // channel 6, 5kHz, 8-bit resolution
    ledcSetup(7, 5000, 8);
    ledcSetup(8, 5000, 8);
    
    ledcAttachPin(_LED_PIN_RED, 6);
    ledcAttachPin(_LED_PIN_GREEN, 7);
    ledcAttachPin(_LED_PIN_BLUE, 8);
    setRGBColor(_COLOR_WHITE);
    Serial.println("RGB LED initialized");
}

void GestureGripJoints::setRGBColor(RGBColor color) {
    setRGBColorPWM(color.r, color.g, color.b);
}

void GestureGripJoints::setRGBColorPWM(uint8_t r, uint8_t g, uint8_t b) {
    ledcWrite(6, (uint8_t)(r * _LED_BRIGHTNESS)); // red
    ledcWrite(7, (uint8_t)(g * _LED_BRIGHTNESS)); // green
    ledcWrite(8, (uint8_t)(b * _LED_BRIGHTNESS)); // blue
}