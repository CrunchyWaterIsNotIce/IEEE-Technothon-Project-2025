#include <Arduino.h>

#define LED_PIN 5

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  // initial light up sequence
  for(int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  delay(1000);
}

void loop() {
  // continuous blinking
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}