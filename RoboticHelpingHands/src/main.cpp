#include <Arduino.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <SparkFun_APDS9960.h>

SparkFun_APDS9960 gesture_sensor = SparkFun_APDS9960();

// found in SparkFun_APDS9960.h
#define APDS9960_PDATA   0x9C  // proximity data register
#define APDS9960_GFLVL   0xAE  // FIFO level, how much data is waiting
#define APDS9960_GFIFO_U 0xFC  // FIFO UP value
#define APDS9960_GFIFO_D 0xFD  // FIFO DOWN value
#define APDS9960_GFIFO_L 0xFE  // FIFO LEFT value
#define APDS9960_GFIFO_R 0xFF  // FIFO RIGHT value

const int SAMPLE_TIME = 2000; // 2 s
const String NAME_OF_GESTURE = "double_shake";
const int SAMPLE_DELAY = 50; // 50 ms
const int NUM_SAMPLES = SAMPLE_TIME / SAMPLE_DELAY;

bool readRegister(uint8_t, uint8_t&);
void handleTrainingData();

void setup() {
  // Initializes serial port number for Serial Monitor
  Serial.begin(115200);

  // Initializes wires to I2C pins w/ fast speed (to keep I2C stable)
  Wire.begin(21, 22, 100000);
  
  if (gesture_sensor.init()) Serial.println("apds is initialized"); // initializes sensor
  else Serial.println("apds failed to initialize");

  // Tunes sensors down to stop saturation, default is too high
  gesture_sensor.setGestureGain(GGAIN_2X);
  gesture_sensor.setGestureLEDDrive(LED_DRIVE_25MA);

  if (gesture_sensor.enableGestureSensor(false)) Serial.println("gesture is initialized"); // initializes gesture
  else Serial.println("gesture failed to initialize");

  // Prompts custom gesture
  Serial.print("Press ~ to run record custom gesture: ");
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '~') {
      while(Serial.available() > 0) Serial.read();

      Serial.println();
      Serial.printf("Now recording gesture, %s, for 2 seconds in...\n", NAME_OF_GESTURE);
      for (int countdown = 3; countdown > 0; countdown--) {
        Serial.printf("%d\n", countdown);
        delay(1000);
      }
      Serial.println("GO");
      delay(1000);

      Serial.println();
      Serial.println("proximity,up,down,left,right");
      // for (int n = 0; n < NUM_SAMPLES; n++) {
      //   handleTrainingData();
      //   delay(SAMPLE_DELAY);
      // }

      // uses non-blocking timing from arduino, no delays except for sampling rate
      unsigned long startTime = millis();
      int sampleCount = 0;
      
      while (sampleCount < NUM_SAMPLES) {
        handleTrainingData();
        sampleCount++;
        
        unsigned long targetTime = startTime + (sampleCount * SAMPLE_DELAY);
        while (millis() < targetTime) {}
      }

      Serial.println();
      Serial.print("Press ~ to rerun recording: ");
    }
  }
}


bool readRegister(uint8_t reg, uint8_t &val) {
  Wire.beginTransmission(APDS9960_I2C_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) { // no transmission sends a restart
    return false; // sensor ACK error
  }

  Wire.requestFrom((uint8_t)APDS9960_I2C_ADDR, (uint8_t)1);

  if (Wire.available()) {
    val = Wire.read();
    return true;
  }
  return false;
}

void handleTrainingData() {
  uint8_t fifo_level = 0;
  uint8_t prox_val = 0;
  uint8_t u_val, d_val, l_val, r_val;

  if ( !readRegister(APDS9960_PDATA, prox_val) ) {
    Serial.println("Error reading proximity");
    return;
  }
  
  if ( !readRegister(APDS9960_GFLVL, fifo_level) ) {
    Serial.println("Error reading FIFO level");
    return;
  }

  if ( fifo_level > 0 ) {
    
    for ( int i = 0; i < fifo_level; i++ ) { // drain FIFO
      
      if (!readRegister(APDS9960_GFIFO_U, u_val) ||
          !readRegister(APDS9960_GFIFO_D, d_val) ||
          !readRegister(APDS9960_GFIFO_L, l_val) ||
          !readRegister(APDS9960_GFIFO_R, r_val) 
        ) {
        Serial.println("Error reading FIFO data");
        break; 
      }
      
      // proximity, up, down, left, right
      Serial.printf("%u,%u,%u,%u,%u\n", prox_val, u_val, d_val, l_val, r_val);
    }
  }

}
