#include "gesture_grip_sensors.h"

GestureGripSensors::GestureGripSensors() :
    _i2c_left(0),
    _i2c_right(1),
    _left_apds(&_i2c_left),
    _right_apds(&_i2c_right)
{}

bool GestureGripSensors::initialize() {
    _i2c_left.begin(_LEFT_SDA_PIN, _LEFT_SCL_PIN, 100000); // hopefully fastest
    _i2c_right.begin(_RIGHT_SDA_PIN, _RIGHT_SCL_PIN, 100000);
    
    _i2c_left.setTimeout(200);
    _i2c_right.setTimeout(200);

    bool left_init = _left_apds.init();
    bool right_init = _right_apds.init();
    
    if (left_init) Serial.println("Left sensor initialized");
    else Serial.println("Left sensor failed");

    if (right_init) Serial.println("Right sensor initialized");
    else Serial.println("Right sensor failed");

    // gained from sparkfun apds, TESTED
    _left_apds.setGestureGain(GGAIN_2X);
    _left_apds.setGestureLEDDrive(LED_DRIVE_25MA);
    _left_apds.setProximityGain(PGAIN_2X);
    _left_apds.setGestureEnterThresh(60);
    _left_apds.setGestureExitThresh(50);

    _right_apds.setGestureGain(GGAIN_2X);
    _right_apds.setGestureLEDDrive(LED_DRIVE_25MA);
    _right_apds.setProximityGain(PGAIN_2X);
    _right_apds.setGestureEnterThresh(60);
    _right_apds.setGestureExitThresh(50);

    if (_left_apds.enableProximitySensor(false)) Serial.println("Left proximity enabled");
    else Serial.println("Left proximity failed");

    if (_right_apds.enableProximitySensor(false)) Serial.println("Right proximity enabled");
    else Serial.println("Right proximity failed");

    if (_left_apds.enableGestureSensor(false)) Serial.println("Left gesture enabled");
    else Serial.println("Left gesture failed");

    if (_right_apds.enableGestureSensor(false)) Serial.println("Right gesture enabled");
    else Serial.println("Right gesture failed");

    return left_init && right_init;
}

bool GestureGripSensors::leftGestureAvailable() {
    return _left_apds.isGestureAvailable();
}

bool GestureGripSensors::rightGestureAvailable() {
    return _right_apds.isGestureAvailable();
}

int GestureGripSensors::readLeftGesture() {
    return readGestureNonBlocking(_left_apds);
}

int GestureGripSensors::readRightGesture() {
    return readGestureNonBlocking(_right_apds);
}

void GestureGripSensors::clearStartupGestures() {
    Serial.println("Clearing startup gestures...");
    delay(1000);
    
    for (int i = 0; i < 10; i++) {
        if (_left_apds.isGestureAvailable()) {
            _left_apds.readGesture();
            delay(50);
        }
        if (_right_apds.isGestureAvailable()) {
            _right_apds.readGesture();
            delay(50);
        }
        delay(50);
    }
    
    Serial.println("Sensors ready!");
}

int GestureGripSensors::readGestureNonBlocking(SparkFun_APDS9960& apds) {
    int gesture = apds.readGesture();
    return (gesture == -1 || gesture == 0) ? DIR_NONE : gesture;
}