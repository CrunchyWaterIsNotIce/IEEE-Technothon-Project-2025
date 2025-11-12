#ifndef SERVO_UTILITIES_H
#define SERVO_UTILITIES_H

#include <array>
#include <Arduino.h>
#include <ESP32Servo.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

struct ServoLimits {
    int min_angle;
    int max_angle;
};

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
     * @brief   gets if the current servo is moving
     * @returns boolean of moving servo
     */
    bool get_is_moving();
    
    /**
     * @brief   moves servo to target angle with easing
     * @param[in]   type: easing type ("sin", "cos", "linear")
     * @param[in]   to_angle: target angle
     * @param[in]   steps_per_degree: subdivisions per degree (default 1, higher = smoother)
     * @returns none
     */
    void move_to(const char* type, int to_angle, int steps_per_degree = 1);
    

    /**
     * @brief   stops all joint movement of servos
     * @returns none
     */
    void stop_movement();

private:
    Servo _servo;
    int _signalPin;
    int _timerNum;
    int _currentAngle;
    bool _isAttached;
    std::array<int, 2> _boundaries;
    static constexpr int _tolerance = 3;
    static constexpr int _movement_deadzone = 5;
    
    TaskHandle_t _moveTaskHandle;
    bool _is_moving;
    
    static void moveTaskWrapper(void* parameter);
    void moveTask(int target_angle, const char* easing_type, int steps_per_degree);
};

#endif