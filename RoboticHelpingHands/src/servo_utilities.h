#ifndef SERVO_UTILITIES_H
#define SERVO_UTILITIES_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoController {
public:
    ServoController();

    /**
     * @brief   initializes servo motor properties for attachment
     * @param[in]   pin: SIGNAL pin
     * @param[in]   timer: allocated timer number
     * @param[in]   angle: angle to initially set servo gear
     * @param[in]   to_attach: marks servo as attached or not
     * @returns successful attachment of servo
     */
    bool attach(int pin, int timer, int angle, bool to_attach);
    /**
     * @brief   writes servo angle within acceptable bounds of 0-180 degrees
     * @param[in]   angle: angle to move servo gear
     * @returns none
     */
    void safe_servo_write(int angle);

    /**
     * @brief   gets current angle of servo
     * @returns current angle of servo
     */
    int get_current_angle();

private:
    Servo _servo;

    int _signalPin;
    int _timerNum;
    int _currentAngle;
    bool _isAttached;
};

#endif