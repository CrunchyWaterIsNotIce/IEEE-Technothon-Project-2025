#ifndef SERVO_UTILITIES_H
#define SERVO_UTILITIES_H

#include <array>
#include <Arduino.h>
#include <ESP32Servo.h>

class ServoController {
public:
    ServoController();

    /**
     * @brief   initializes servo motor properties for attachment
     * @param[in]   pin: SIGNAL pin
     * @param[in]   timer: allocated timer number
     * @param[in]   to_attach: marks servo as attached or not
     * @param[in]   angle: angle to initially set servo gear
     * @param[in]   boundary: angle boundaries of start and end
     * @returns successful attachment of servo
     */
    bool attach(int pin, int timer, bool to_attach, int angle, std::array<int, 2> boundary);
    
    /**
     * @brief   writes servo angle within acceptable bounds of set boundary
     * @param[in]   angle: angle to move servo gear
     * @returns none
     */
    void safe_servo_write(int angle);

    /**
     * @brief   gets current angle of servo
     * @returns current angle of servo
     */
    int get_current_angle();

    /**
     * @brief   moves servo to angle with a set movement function
     * @param[in]   angle: angle to move servo gear
     * @returns none
     */
    void move_to(String type, int to_angle);

private:
    Servo _servo;

    int _tolerance = 2;
    int _signalPin;
    int _timerNum;
    int _currentAngle;
    bool _isAttached;
    std::array<int, 2> _boundaries;
};

#endif