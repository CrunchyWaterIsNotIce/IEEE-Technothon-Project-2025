#ifndef GESTURE_GRIP_JOINTS_H
#define GESTURE_GRIP_JOINTS_H

#include <Arduino.h>
#include "servo_utilities.h"

/**
 * @brief   manages all servo joints for robotic arm, LED Feedback is here
 */
class GestureGripJoints {
public:
    GestureGripJoints();

    /**
     * @brief   initializes all servo motors and RGB LED
     * @returns true if all servos attached successfully
     */
    bool initialize();

    /**
     * @brief   moves entire arm to upright position
     * @returns none
     */
    void moveToUpright();

    /**
     * @brief   moves entire arm to downward position
     * @returns none
     */
    void moveToDownward();

    /**
     * @brief   adjusts selected servo by angle increment
     * @param[in]   servo_index: index of servo to adjust from 0 to 4
     * @param[in]   increment: degrees to add
     * @returns none
     */
    void adjustServo(int servo_index, int increment);

    /**
     * @brief   gets current angle of specified servo
     * @param[in]   servo_index: index of servo from 0 to 4
     * @returns current angle in degrees, or -1 if invalid
     */
    int getServoAngle(int servo_index);

    /**
     * @brief   gets label name of specified servo
     * @param[in]   servo_index: index of servo from 0 to 4
     * @returns pointer to servo label string
     */
    const char* getServoLabel(int servo_index);

    /**
     * @brief   updates LED based on control state
     * @param[in]   state: 0 = DIRECT; 1 = SELECT; 2 = ADJUST
     * @param[in]   selected_servo: index of currently selected servo, -1 if none burh
     * @returns none
     */
    void updateLED(int state, int selected_servo);

    /**
     * @brief   gets total number of controllable servos
     * @returns servo count
     */
    int getServoCount() const { return _SERVO_COUNT; }
    

private:
    static const int _SERVO_COUNT = 5;

    // Servo controllers
    ServoController _servo_base;
    ServoController _servo_middle;
    ServoController _servo_cross;
    ServoController _servo_left;
    ServoController _servo_right;

    // Servo pin definitions
    const int _PIN_BASE = 27;
    const int _PIN_MIDDLE = 26;
    const int _PIN_CROSS = 25;
    const int _PIN_LEFT = 33;
    const int _PIN_RIGHT = 32;

    // RGB LED pins
    const int _LED_PIN_RED = 23;
    const int _LED_PIN_GREEN = 19;
    const int _LED_PIN_BLUE = 18;
    const float _LED_BRIGHTNESS = 0.3;

    // Servo reference array
    ServoController* _servoRefs[5];
    const char* _servoLabels[5] = {"BASE", "MIDDLE", "CROSS", "LEFT", "RIGHT"};

    // RGB color definitions
    struct RGBColor {
        uint8_t r, g, b;
    };

    const RGBColor _servoColors[5] = {
        {150, 0, 255},    // base purple
        {50, 232, 133},   // middle green
        {255, 190, 0},    // cross orange
        {0, 0, 255},      // left blue
        {255, 0, 0}       // right red
    };

    const RGBColor _COLOR_WHITE = {255, 255, 255};

    // LED state tracking
    bool _ledState;
    unsigned long _lastBlink;
    const unsigned long _BLINK_INTERVAL = 500;

    /**
     * @brief   initializes RGB LED PWM channels
     * @returns none
     */
    void initializeLED();

    /**
     * @brief   sets RGB LED color
     * @param[in]   color: RGB color structure
     * @returns none
     */
    void setRGBColor(RGBColor color);

    /**
     * @brief   sets RGB LED color with individual values
     * @param[in]   r: red value between 0 to 255
     * @param[in]   g: green value between 0 to 255
     * @param[in]   b: blue value between 0 to 255
     * @returns none
     */
    void setRGBColorPWM(uint8_t r, uint8_t g, uint8_t b);

};

#endif