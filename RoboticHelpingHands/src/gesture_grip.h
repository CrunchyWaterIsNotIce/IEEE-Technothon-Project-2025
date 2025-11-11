#ifndef GESTURE_GRIP_H
#define GESTURE_GRIP_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "gesture_grip_sensors.h"
#include "gesture_grip_joints.h"

/**
 * @brief   Main controller for gesture-controlled robotic arm
 */
class GestureGrip {
public:
    GestureGrip();

    /**
     * @brief   Initializes all subsystems (sensors, servos, tasks)
     * @returns true if initialization successful
     */
    bool initialize();

    /**
     * @brief   Starts FreeRTOS tasks for gesture and servo control
     * @returns none
     */
    void start();

    /**
     * @brief   Main update loop (call from Arduino loop)
     * @returns none
     */
    void update();

private:
    // Control state machine
    enum ControlState {
        STATE_DIRECT = 0,      // Normal gesture control
        STATE_SELECT_SERVO,    // Selecting which servo to control
        STATE_ADJUST_SERVO     // Adjusting selected servo
    };

    volatile ControlState _control_state;
    int _selected_servo_index;
    const int _SERVO_STEP_DEGREES = 5;

    // Subsystem controllers
    GestureGripSensors _sensors;
    GestureGripJoints _joints;

    // FreeRTOS components
    TaskHandle_t _gestureTaskHandle;
    TaskHandle_t _servoTaskHandle;
    TaskHandle_t _ledTaskHandle;
    QueueHandle_t _gestureQueue;

    struct GestureEvent {
        int gesture;
        bool isLeft;
    };

    // Timing control
    unsigned long _lastStateChange;
    const unsigned long _STATE_CHANGE_DEBOUNCE = 1000;

    /**
     * @brief   FreeRTOS task for reading gestures
     * @param[in]   parameter: pointer to GestureGrip instance
     * @returns none
     */
    static void gestureTaskWrapper(void* parameter);

    /**
     * @brief   FreeRTOS task for controlling servos
     * @param[in]   parameter: pointer to GestureGrip instance
     * @returns none
     */
    static void servoTaskWrapper(void* parameter);

    /**
     * @brief   FreeRTOS task for controlling LED
     * @param[in]   parameter: pointer to GestureGrip instance
     * @returns none
     */
    static void ledTaskWrapper(void* parameter);

    /**
     * @brief   Main gesture reading loop
     * @returns none
     */
    void gestureTask();

    /**
     * @brief   Main servo control loop
     * @returns none
     */
    void servoTask();

    /**
     * @brief   Main LED update loop
     * @returns none
     */
    void ledTask();

    /**
     * @brief   Advances to next control state
     * @returns none
     */
    void advanceControlState();

    /**
     * @brief   Announces currently selected servo
     * @returns none
     */
    void announceSelectedServo();

    /**
     * @brief   Handles gesture in direct control mode
     * @param[in]   event: gesture event structure
     * @returns none
     */
    void handleDirectGesture(const GestureEvent& event);

    /**
     * @brief   Handles gesture in servo selection mode
     * @param[in]   gesture: gesture direction constant
     * @returns none
     */
    void handleSelectionGesture(int gesture);

    /**
     * @brief   Handles gesture in servo adjustment mode
     * @param[in]   gesture: gesture direction constant
     * @returns none
     */
    void handleAdjustGesture(int gesture);

    /**
     * @brief   Prints gesture to serial monitor
     * @param[in]   gesture: gesture direction constant
     * @param[in]   sensor_name: name of sensor that detected gesture
     * @returns none
     */
    void handleGesture(int gesture, const char* sensor_name);
};

#endif