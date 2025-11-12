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
    initializeLED(); // initialize the leds

    Serial.println("Attaching servos with soft start...");
    
    bool success = true;
    // Attach all servos starting at 0 degrees (safe position)
    success &= _servo_base.attach(_PIN_BASE, 1, true, 0, {20, 80});
    delay(200);  // Give servo time to settle
    success &= _servo_middle.attach(_PIN_MIDDLE, 2, true, 0, {0, 80});
    delay(200);
    success &= _servo_cross.attach(_PIN_CROSS, 3, true, 0, {0, 180});
    delay(200);
    success &= _servo_left.attach(_PIN_LEFT, 4, true, 0, {0, 170});
    delay(200);
    success &= _servo_right.attach(_PIN_RIGHT, 5, true, 0, {0, 170});
    delay(200);  // Extra delay before movement

    Serial.println("Servos attached and stabilized");
    return success;
}

bool GestureGripJoints::waitForServos(unsigned long timeout_ms) {
    unsigned long startTime = millis();
    
    Serial.println("Waiting for all servos to reach target...");
    
    while (millis() - startTime < timeout_ms) {
        bool allStopped = true;
        
        // Check if all servos have stopped moving
        if (_servo_base.get_is_moving()) allStopped = false;
        if (_servo_middle.get_is_moving()) allStopped = false;
        if (_servo_cross.get_is_moving()) allStopped = false;
        if (_servo_left.get_is_moving()) allStopped = false;
        if (_servo_right.get_is_moving()) allStopped = false;
        
        if (allStopped) {
            Serial.println("All servos reached their targets!");
            Serial.printf("  Base: %d°\n", _servo_base.get_current_angle());
            Serial.printf("  Middle: %d°\n", _servo_middle.get_current_angle());
            Serial.printf("  Cross: %d°\n", _servo_cross.get_current_angle());
            Serial.printf("  Left: %d°\n", _servo_left.get_current_angle());
            Serial.printf("  Right: %d°\n", _servo_right.get_current_angle());
            return true;
        }
        
        // Print progress every second
        if ((millis() - startTime) % 1000 < 50) {
            Serial.printf("Still moving... (%.1fs elapsed)\n", (millis() - startTime) / 1000.0);
        }
        
        delay(50);
    }
    
    Serial.println("WARNING: Timeout waiting for servos!");
    return false;
}

void GestureGripJoints::stopAllMovements() {
    Serial.println("Stopping all servo movement tasks...");
    
    // Stop all servo movement tasks by setting them to their current position
    _servo_base.stop_movement();
    _servo_middle.stop_movement();
    _servo_cross.stop_movement();
    _servo_left.stop_movement();
    _servo_right.stop_movement();
    
    vTaskDelay(pdMS_TO_TICKS(100));
}

void GestureGripJoints::lockOtherServos(int servo_index) {
    // Actively hold all other servos at their current position
    // This prevents jitter when one servo moves
    for (int i = 0; i < _SERVO_COUNT; i++) {
        if (i != servo_index) {
            int current_angle = _servoRefs[i]->get_current_angle();
            _servoRefs[i]->safe_servo_write(current_angle);
        }
    }
}

void GestureGripJoints::adjustServo(int servo_index, int increment) {
    if (servo_index < 0 || servo_index >= _SERVO_COUNT) return;
    
    // First, lock all other servos at their current position
    lockOtherServos(servo_index);
    
    ServoController* servo = _servoRefs[servo_index];
    int current = servo->get_current_angle();
    int target = current + increment;
    
    // Get boundaries from servo's attached boundaries
    std::array<int, 2> boundaries = {{0, 180}};
    switch(servo_index) {
        case 0: boundaries = {{20, 80}}; break;   // base
        case 1: boundaries = {{0, 80}}; break;    // middle
        case 2: boundaries = {{0, 180}}; break;   // cross
        case 3: boundaries = {{0, 170}}; break;   // left
        case 4: boundaries = {{0, 170}}; break;   // right
    }
    target = constrain(target, boundaries[0], boundaries[1]);
    
    Serial.printf("%s: %d° -> %d° (step: %+d°)\n",
                  _servoLabels[servo_index],
                  current,
                  target,
                  increment);
    
    servo->safe_servo_write(target);
    
    // Re-lock other servos after adjustment to maintain stability
    vTaskDelay(pdMS_TO_TICKS(10));  // Small delay for servo to start moving
    lockOtherServos(servo_index);
}

void GestureGripJoints::moveToUpright(int steps_per_degree) {
    Serial.println("Moving to UPWARD position...");
    _servo_base.move_to("cos", 50, steps_per_degree);
    vTaskDelay(pdMS_TO_TICKS(300));
    _servo_middle.move_to("cos", 60, steps_per_degree);  
    vTaskDelay(pdMS_TO_TICKS(300));
    _servo_cross.move_to("cos", 90, steps_per_degree);
    vTaskDelay(pdMS_TO_TICKS(200));
    _servo_left.move_to("cos", 85, steps_per_degree);
    vTaskDelay(pdMS_TO_TICKS(50));
    _servo_right.move_to("cos", 85, steps_per_degree);
}

void GestureGripJoints::moveToDownward(int steps_per_degree) {
    Serial.println("Moving to DOWNWARD position...");
    _servo_base.move_to("cos", 75, steps_per_degree);
    vTaskDelay(pdMS_TO_TICKS(300));
    _servo_middle.move_to("cos", 100, steps_per_degree);
    vTaskDelay(pdMS_TO_TICKS(300));
    _servo_cross.move_to("cos", 0, steps_per_degree);  
    vTaskDelay(pdMS_TO_TICKS(200));
    _servo_left.move_to("cos", 0, steps_per_degree);
    vTaskDelay(pdMS_TO_TICKS(50));
    _servo_right.move_to("cos", 0, steps_per_degree);
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
                setRGBColorPWM(0, 0, 0);
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