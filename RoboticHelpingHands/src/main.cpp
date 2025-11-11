#include <Arduino.h>
#include "gesture_grip.h"

GestureGrip gestureGrip;

void setup() {
    Serial.begin(115200);
    
    if (!gestureGrip.initialize()) {
        Serial.println("FATAL: Failed to initialize GestureGrip!");
        while (1) delay(1000);
    }
    
    gestureGrip.start();
}

void loop() {
    gestureGrip.update();
}