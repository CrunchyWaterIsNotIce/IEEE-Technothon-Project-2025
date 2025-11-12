#include "servo_utilities.h"
#include <math.h>

ServoController::ServoController() :
    _signalPin(-1),
    _timerNum(-1),
    _currentAngle(0),
    _isAttached(false),
    _boundaries{0, 180},
    _moveTaskHandle(NULL),
    _is_moving(false)
{}

bool ServoController::attach(int pin, int timer, bool to_attach, int angle, std::array<int, 2> boundary) {
    if (_isAttached) return false;

    _signalPin = pin;
    _timerNum = timer;
    _isAttached = to_attach;
    _currentAngle = angle;
    _boundaries = boundary;
    
    if (timer >= 0) ESP32PWM::allocateTimer(timer);
    _servo.setPeriodHertz(50);  // Changed: 100 -> 50 (standard servo frequency)
    _servo.attach(pin, 500, 2500);

    safe_servo_write(angle);
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

// NEW: Task-based asynchronous movement with cleanup
void ServoController::move_to(const char* type, int to_angle, int steps_per_degree) {
    if (!_isAttached) return;
    
    // Check if movement is within deadzone - if so, just set directly
    to_angle = constrain(to_angle, _boundaries[0], _boundaries[1]);
    int movement_needed = abs(to_angle - _currentAngle);
    
    if (movement_needed < _movement_deadzone) {
        // Movement too small, just set directly without creating task
        safe_servo_write(to_angle);
        return;
    }
    
    // Stop any existing movement task
    if (_moveTaskHandle != NULL) {
        _is_moving = false;  // Signal task to stop
        vTaskDelay(pdMS_TO_TICKS(50));  // Give it time to stop
        vTaskDelete(_moveTaskHandle);
        _moveTaskHandle = NULL;
    }
    
    _is_moving = true;
    
    // Create parameter struct
    struct MoveParams {
        ServoController* controller;
        int target;
        int steps_multiplier;
        char easing[10];
    };

    MoveParams* params = new MoveParams;
    params->controller = this;
    params->target = to_angle;
    params->steps_multiplier = steps_per_degree;
    strncpy(params->easing, type, 9);
    params->easing[9] = '\0';

    // Create FreeRTOS task
    xTaskCreate(
        moveTaskWrapper,
        "ServoMove",
        2048,
        params,
        1,
        &_moveTaskHandle
    );
}

// NEW: Static wrapper to call member function
void ServoController::moveTaskWrapper(void* parameter) {
    struct MoveParams {
        ServoController* controller;
        int target;
        int steps_multiplier;
        char easing[10];
    };

    MoveParams* params = static_cast<MoveParams*>(parameter);
    params->controller->moveTask(params->target, params->easing, params->steps_multiplier);
    
    // Cleanup
    params->controller->_is_moving = false;
    params->controller->_moveTaskHandle = NULL;
    
    delete params;
    vTaskDelete(NULL);
}

void ServoController::moveTask(int target_angle, const char* easing_type, int steps_per_degree) {
    target_angle = constrain(target_angle, _boundaries[0], _boundaries[1]);
    
    int start_angle = _currentAngle;
    int angle_distance = abs(target_angle - start_angle);
    
    // Don't move if already within tolerance
    if (angle_distance < _movement_deadzone) {
        safe_servo_write(target_angle);
        _is_moving = false;
        _moveTaskHandle = NULL;
        return;
    }

    // Calculate total steps with subdivisions
    int total_steps = angle_distance * steps_per_degree;
    const int step_delay_ms = 20;
    int direction = (target_angle > start_angle) ? 1 : -1;
    int last_written_angle = start_angle;

    // Loop with interrupt checking
    for (int step = 0; step <= total_steps && _is_moving; step++) {
        float progress = (float)step / total_steps;
        float eased_progress = progress;

        if (strcmp(easing_type, "sin") == 0) {
            eased_progress = sin(progress * PI / 2);
        } else if (strcmp(easing_type, "cos") == 0) {
            eased_progress = 1.0 - cos(progress * PI / 2);
        }

        int new_angle = start_angle + (int)(eased_progress * angle_distance * direction);
        
        // Only write if the angle changed enough to avoid micro jitters
        if (abs(new_angle - last_written_angle) >= _tolerance) {
            safe_servo_write(new_angle);
            last_written_angle = new_angle;
        }
        
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));  // Non-blocking delay
    }

    // Set final position if not interrupted
    if (_is_moving) {
        safe_servo_write(target_angle);
    }
    
    _is_moving = false;
    _moveTaskHandle = NULL;
}

void ServoController::stop_movement() {
    if (_moveTaskHandle != NULL) {
        _is_moving = false;
        vTaskDelay(pdMS_TO_TICKS(20));
        vTaskDelete(_moveTaskHandle);
        _moveTaskHandle = NULL;
    }
}

int ServoController::get_current_angle() {
    if (!_isAttached) return -1;
    return _currentAngle;
}

bool ServoController::get_is_moving() {
    return _is_moving;
}
