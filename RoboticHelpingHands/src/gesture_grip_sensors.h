#ifndef GESTURE_GRIP_SENSORS_H
#define GESTURE_GRIP_SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>

/**
 * @brief   manages the dual APDS-9960 gesture sensors for robotic arm
 */
class GestureGripSensors {
public:
    GestureGripSensors();

    /**
     * @brief   initializes both I2C buses and APDS-9960 sensors
     * @returns true if both sensors initialized successfully
     */
    bool initialize();

    /**
     * @brief   checks if left sensor has gesture available
     * @returns true if gesture is ready to be read
     */
    bool leftGestureAvailable();

    /**
     * @brief   checks if right sensor has gesture available
     * @returns true if gesture is ready to be read
     */
    bool rightGestureAvailable();

    /**
     * @brief   reads gesture from left sensor in nonblocking mode
     * @returns gesture direction constant (DIR_UP, DIR_DOWN, ..., or DIR_NONE)
     */
    int readLeftGesture();

    /**
     * @brief   reads gesture from right sensor in non-blocking mode
     * @returns gesture direction constant (DIR_UP, DIR_DOWN, ..., or DIR_NONE)
     */
    int readRightGesture();

    /**
     * @brief   clears any pending gestures during startup
     * @returns none
     */
    void clearStartupGestures();

private:
    TwoWire _i2c_left;
    TwoWire _i2c_right;
    SparkFun_APDS9960 _left_apds;
    SparkFun_APDS9960 _right_apds;

    const int _LEFT_SCL_PIN = 22;
    const int _LEFT_SDA_PIN = 21;
    const int _RIGHT_SCL_PIN = 17;
    const int _RIGHT_SDA_PIN = 16;

    /**
     * @brief   reads gesture in non-blocking mode with error handling
     * @param[in]   apds: reference to APDS-9960 sensor
     * @returns gesture direction or DIR_NONE on error
     */
    int readGestureNonBlocking(SparkFun_APDS9960& apds);
};

#endif