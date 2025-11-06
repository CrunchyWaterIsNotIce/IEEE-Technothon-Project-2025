#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>

TwoWire I2C_left = TwoWire(0);
TwoWire I2C_right = TwoWire(1);

SparkFun_APDS9960 left_apds = SparkFun_APDS9960();
SparkFun_APDS9960 right_apds = SparkFun_APDS9960();

const int LEFT_SCL_PIN = 17;
const int LEFT_SDA_PIN = 16;
const int LEFT_INTERUPT_PIN = 23;
volatile bool left_isr_flag = false;

const int RIGHT_SCL_PIN = 22;
const int RIGHT_SDA_PIN = 21;
const int RIGHT_INTERUPT_PIN = 19;
volatile bool right_isr_flag = false;

void handleGesture(SparkFun_APDS9960&);
void IRAM_ATTR leftInterruptRoutine();
void IRAM_ATTR rightInterruptRoutine();
 
void setup() {
    Serial.begin(115200);
    I2C_left.begin(LEFT_SDA_PIN, LEFT_SCL_PIN, 100000);
    I2C_right.begin(RIGHT_SDA_PIN, RIGHT_SCL_PIN, 100000);

    pinMode(LEFT_INTERUPT_PIN, INPUT_PULLUP);
    pinMode(RIGHT_INTERUPT_PIN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(LEFT_INTERUPT_PIN), leftInterruptRoutine, FALLING);
    attachInterrupt(digitalPinToInterrupt(RIGHT_INTERUPT_PIN), rightInterruptRoutine, FALLING);

    if (!left_apds.init()) Serial.println("Left sensor failed to initialize");
    if (!right_apds.init()) Serial.println("Right sensor failed to initialize");

    if (!left_apds.enableGestureSensor()) Serial.println("Left sensor gesture unavailable");
    if (!right_apds.enableGestureSensor()) Serial.println("Right sensor gesture unavailable");
}

void loop() {

}

void handleGesture(SparkFun_APDS9960& apds, const char* sensor_name) {
    if (apds.isGestureAvailable()) {
        Serial.print(sensor_name);
        Serial.print(": \n");
        switch (apds.readGesture()) {
            case DIR_UP:
                Serial.println("swiped UP");
                break;
            case DIR_DOWN:
                Serial.println("swiped DOWN");
                break;
            case DIR_LEFT:
                Serial.println("swiped LEFT");
                break;
            case DIR_RIGHT:
                Serial.println("swiped RIGHT");
                break;
            case DIR_FAR:
                Serial.println("swiped FAR");
                break;
            case DIR_NEAR:
                Serial.println("swiped NEAR");
                break;
            default:
                Serial.println("none");
        }
    }
}

void IRAM_ATTR leftInterruptRoutine() {
    left_isr_flag = true;
}

void IRAM_ATTR rightInterruptRoutine() {
    right_isr_flag = true;
}