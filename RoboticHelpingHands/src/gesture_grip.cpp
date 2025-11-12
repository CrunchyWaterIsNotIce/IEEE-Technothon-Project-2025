#include "gesture_grip.h"
#include <SparkFun_APDS9960.h>

GestureGrip::GestureGrip() :
    _control_state(STATE_DIRECT),
    _selected_servo_index(-1),
    _gestureTaskHandle(NULL),
    _servoTaskHandle(NULL),
    _ledTaskHandle(NULL),
    _stabilizerTaskHandle(NULL),
    _gestureQueue(NULL),
    _lastStateChange(0)
{}

bool GestureGrip::initialize() {
    Serial.println("Initializing LEDS, Sensors, and Individual Servos...");
    
    // Initialize joints first
    if (!_joints.initialize()) {
        Serial.println("Failed to initialize joints!");
        return false;
    }
    
    Serial.println("Waiting for servo power stabilization...");
    delay(500);  // half second for servos to stabilize
    Serial.println("Moving to upright position...");
    _joints.moveToUpright(3);
    
    // Wait for all servos to finish moving
    if (!_joints.waitForServos(8000)) {
        Serial.println("WARNING: Some servos may not have reached target position");
    }
    
    // Stop all movement tasks after initialization completes
    Serial.println("Stopping initialization tasks...");
    _joints.stopAllMovements();
    delay(100);  // small delay to ensure tasks are killed
    
    Serial.println("✓ Arm erected and stabilized");
    
    // Initialize sensors second
    if (!_sensors.initialize()) {
        Serial.println("Failed to initialize sensors!");
        return false;
    }
    
    // Clear startup gestures
    _sensors.clearStartupGestures();
    
    Serial.println("=== Initialized LEDs, Sensors, and Individual Servos ===");
    
    // Create gesture queue
    _gestureQueue = xQueueCreate(1, sizeof(GestureEvent));
    if (_gestureQueue == NULL) {
        Serial.println("Failed to create gesture queue!");
        return false;
    }
    
    return true;
}

void GestureGrip::start() {
    // Create gesture task on core 0 (highest priority for I2C)
    xTaskCreatePinnedToCore(
        gestureTaskWrapper,
        "GestureTask",
        4096,
        this,
        2,
        &_gestureTaskHandle,
        0
    );
    
    // Create servo task on core 1
    xTaskCreatePinnedToCore(
        servoTaskWrapper,
        "ServoTask",
        4096,
        this,
        1,
        &_servoTaskHandle,
        1
    );
    
    // Create LED task on core 1
    xTaskCreatePinnedToCore(
        ledTaskWrapper,
        "LEDTask",
        2048,
        this,
        1,
        &_ledTaskHandle,
        1
    );

    // Create stabilizer task on core 1 (low priority)
    xTaskCreatePinnedToCore(
        stabilizerTaskWrapper,
        "StabilizerTask",
        2048,
        this,
        0,
        &_stabilizerTaskHandle,
        1
    );

    Serial.println("Initialized both APDS and Servo on separate cores.");
    Serial.println("\n=== CONTROL MODES ===");
    Serial.println("DIRECT MODE: White LED - Swipe gestures control arm");
    Serial.println("NEAR/FAR gesture: Enter servo selection mode\n");
}

void GestureGrip::update() {
    vTaskDelay(pdMS_TO_TICKS(100));
}

void GestureGrip::gestureTaskWrapper(void* parameter) {
    GestureGrip* grip = static_cast<GestureGrip*>(parameter);
    grip->gestureTask();
}

void GestureGrip::servoTaskWrapper(void* parameter) {
    GestureGrip* grip = static_cast<GestureGrip*>(parameter);
    grip->servoTask();
}

void GestureGrip::ledTaskWrapper(void* parameter) {
    GestureGrip* grip = static_cast<GestureGrip*>(parameter);
    grip->ledTask();
}

void GestureGrip::stabilizerTaskWrapper(void* parameter) {
    GestureGrip* grip = static_cast<GestureGrip*>(parameter);
    grip->stabilizerTask();
}

void GestureGrip::gestureTask() {
    vTaskDelay(pdMS_TO_TICKS(2000)); // wait 2 seconds before starting gesture detection
    
    Serial.println("Gesture detection active!");
    
    while (true) {
        int left_gesture = DIR_NONE;
        int right_gesture = DIR_NONE;
        
        // Left Sensor
        if (_sensors.leftGestureAvailable()) {
            left_gesture = _sensors.readLeftGesture();
            
            if (left_gesture == DIR_NEAR || left_gesture == DIR_FAR) {
                left_gesture = DIR_NONE;
            }
            
            // Send gesture to queue ONLY if valid (UP/DOWN/LEFT/RIGHT)
            if (left_gesture != DIR_NONE && left_gesture != -1) {
                GestureEvent event = {left_gesture};
                xQueueSend(_gestureQueue, &event, 0);
                handleGesture(left_gesture, "LEFT");
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
        
        // Right Sensor
        if (_sensors.rightGestureAvailable()) {
            right_gesture = _sensors.readRightGesture();
            
            // Only process NEAR/FAR for state changes
            if (right_gesture == DIR_NEAR || right_gesture == DIR_FAR) {
                if (millis() - _lastStateChange > _STATE_CHANGE_DEBOUNCE) {
                    Serial.println(">>> RIGHT: NEAR/FAR detected - changing state <<<");
                    advanceControlState();
                    _lastStateChange = millis();
                }
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void GestureGrip::servoTask() {
    GestureEvent event;
    
    while (true) {
        if (xQueueReceive(_gestureQueue, &event, pdMS_TO_TICKS(10)) == pdTRUE) {
            if (event.gesture == DIR_NONE || event.gesture == -1) {
                Serial.println("Warning: Invalid gesture in queue, skipping");
                continue;
            }
            
            // Handle gesture based on current state
            switch (_control_state) {
                case STATE_DIRECT:
                    handleDirectGesture(event);
                    break;
                    
                case STATE_SELECT_SERVO:
                    handleSelectionGesture(event.gesture);
                    break;
                    
                case STATE_ADJUST_SERVO:
                    handleAdjustGesture(event.gesture);
                    break;
            }
            
            int flushed = 0;
            while (xQueueReceive(_gestureQueue, &event, 0) == pdTRUE) {
                flushed++;
            }
            if (flushed > 0) {
                Serial.printf("Flushed %d queued gestures\n", flushed);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void GestureGrip::ledTask() {
    while (true) {
        _joints.updateLED((int)_control_state, _selected_servo_index);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void GestureGrip::stabilizerTask() {
    while (true) {
        // Only actively stabilize when in ADJUST_SERVO mode
        if (_control_state == STATE_ADJUST_SERVO && _selected_servo_index >= 0) {
            // Periodically lock other servos to prevent drift/jitter
            _joints.lockOtherServos(_selected_servo_index);
            vTaskDelay(pdMS_TO_TICKS(100));  // Lock every 100ms
        } else {
            // Not adjusting, so don't need to stabilize
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void GestureGrip::advanceControlState() {
    // Resets gesture queue
    xQueueReset(_gestureQueue);
    
    switch (_control_state) {
        case STATE_DIRECT:
            _control_state = STATE_SELECT_SERVO;
            _selected_servo_index = 0;
            Serial.println("\n========================================");
            Serial.println("MODE: SERVO SELECTION");
            Serial.println("Swipe UP/DOWN to select servo");
            Serial.println("========================================");
            announceSelectedServo();
            break;
            
        case STATE_SELECT_SERVO:
            _control_state = STATE_ADJUST_SERVO;
            Serial.println("\n========================================");
            Serial.printf("MODE: ADJUSTING %s SERVO\n", _joints.getServoLabel(_selected_servo_index));
            Serial.println("Swipe UP/DOWN to move servo");
            Serial.println("========================================");
            
            // Lock all other servos when entering adjust mode
            Serial.println("🔒 Locking other servos in place...");
            _joints.lockOtherServos(_selected_servo_index);
            break;
            
        case STATE_ADJUST_SERVO:
        default:
            _control_state = STATE_DIRECT;
            _selected_servo_index = -1;
            Serial.println("\n========================================");
            Serial.println("MODE: DIRECT CONTROL");
            Serial.println("Gestures control the arm");
            Serial.println("========================================");
            break;
    }
}

void GestureGrip::announceSelectedServo() {
    if (_selected_servo_index < 0 || _selected_servo_index >= _joints.getServoCount()) return;
    
    Serial.printf(">>> Selected: [%d] %s @ %d° <<<\n",
                  _selected_servo_index,
                  _joints.getServoLabel(_selected_servo_index),
                  _joints.getServoAngle(_selected_servo_index));
}

void GestureGrip::handleDirectGesture(const GestureEvent& event) {
    // Only LEFT sensor sends gestures, so all gestures are from left sensor
    if (event.gesture == DIR_LEFT) {
        _joints.moveToDownward(1);
    } else if (event.gesture == DIR_RIGHT) {
        _joints.moveToUpright(1);
    }
}

void GestureGrip::handleSelectionGesture(int gesture) {
    if (gesture == DIR_UP) {
        _selected_servo_index = (_selected_servo_index - 1 + _joints.getServoCount()) % _joints.getServoCount();
        announceSelectedServo();
    } else if (gesture == DIR_DOWN) {
        _selected_servo_index = (_selected_servo_index + 1) % _joints.getServoCount();
        announceSelectedServo();
    }
}

void GestureGrip::handleAdjustGesture(int gesture) {
    if (_selected_servo_index < 0 || _selected_servo_index >= _joints.getServoCount()) return;
    
    if (gesture == DIR_UP) {
        _joints.adjustServo(_selected_servo_index, _SERVO_STEP_DEGREES);
    } else if (gesture == DIR_DOWN) {
        _joints.adjustServo(_selected_servo_index, -_SERVO_STEP_DEGREES);
    }
}

void GestureGrip::handleGesture(int gesture, const char* sensor_name) {
    if (gesture == DIR_NEAR || gesture == DIR_FAR) {
        return;
    }
    
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
        default:
            Serial.println("NONE");
    }
}