#include <Arduino.h>
#include <Wire.h>

#include <ESP32Servo.h>
#include <SparkFun_APDS9960.h>

#include "servo_utilities.h"

// Gesture Sensor Properties 
TwoWire I2C_left = TwoWire(0);
TwoWire I2C_right = TwoWire(1);

SparkFun_APDS9960 left_apds = SparkFun_APDS9960(&I2C_left);
SparkFun_APDS9960 right_apds = SparkFun_APDS9960(&I2C_right);

const int LEFT_SCL_PIN = 22;
const int LEFT_SDA_PIN = 21;

const int RIGHT_SCL_PIN = 17;
const int RIGHT_SDA_PIN = 16;

// Servo Properties
#define SIGNAL_PIN_SERVO_BASE 27
#define SIGNAL_PIN_SERVO_MIDDLE 26
#define SIGNAL_PIN_SERVO_CROSS 25
#define SIGNAL_PIN_SERVO_LEFT 33
#define SIGNAL_PIN_SERVO_RIGHT 32
ServoController servo_base;
ServoController servo_middle;
ServoController servo_cross;
ServoController servo_left;
ServoController servo_right;

constexpr int SERVO_COUNT = 5;

// RGB LED Properties
#define LED_PIN_RED 23
#define LED_PIN_GREEN 19
#define LED_PIN_BLUE 18

struct RGBColor {
  uint8_t r, g, b;
};

const RGBColor servoColors[SERVO_COUNT] = {
  {150, 0, 255},     // base is purple
  {50, 232, 133},       // middle is green
  {255, 190, 0},       // cross is orange
  {0, 0, 255},       // left is blue
  {255, 0, 0}        // right is red
};

const RGBColor COLOR_WHITE = {255, 255, 255};
const float LED_BRIGHTNESS = 0.3; // 30%

// Control state machine
enum ControlState {
  STATE_DIRECT = 0,      // normal
  STATE_SELECT_SERVO,    // selecting which servo to control
  STATE_ADJUST_SERVO     // adjusting selected servo
};

volatile ControlState control_state = STATE_DIRECT;
int selected_servo_index = -1;


ServoController* servoRefs[SERVO_COUNT] = { &servo_base, &servo_middle, &servo_cross, &servo_left, &servo_right };
const char* servoLabels[SERVO_COUNT] = { "BASE", "MIDDLE", "CROSS", "LEFT", "RIGHT" };
const int SERVO_STEP_DEGREES = 5;

// Task handles
TaskHandle_t gestureTaskHandle = NULL;
TaskHandle_t servoTaskHandle = NULL;
TaskHandle_t ledTaskHandle = NULL;

// Gesture queue
QueueHandle_t gestureQueue; 
struct GestureEvent {
  int gesture;
  bool isLeft;
};

// Function Declarations
void apdsInitialization();
void servoInitialization();
void ledInitialization();
void handleGesture(int gesture, const char* sensor_name);
int readGestureNonBlocking(SparkFun_APDS9960& apds);
void moveHorizontalArm(String direction);

// FreeRTOS Tasks
void gestureTask(void *parameter);
void servoTask(void *parameter);
void ledTask(void *parameter); 

// LED control functions
void setRGBColor(RGBColor color);
void setRGBColorPWM(uint8_t r, uint8_t g, uint8_t b);

// State machine functions
void advanceControlState();
void announceSelectedServo();
void handleDirectGesture(const GestureEvent& event);
void handleSelectionGesture(int gesture);
void handleAdjustGesture(int gesture);

void setup() {
  Serial.begin(115200);
  
  // queue for gesture events
  gestureQueue = xQueueCreate(1, sizeof(GestureEvent));
  
  ledInitialization();
  apdsInitialization();
  servoInitialization();

  delay(2000);
  
  // gesture task on core 0 (high priority for I2C)
  xTaskCreatePinnedToCore(
    gestureTask,
    "GestureTask",
    4096,
    NULL,
    2,
    &gestureTaskHandle,
    0
  );
  
  // servo task on core 1
  xTaskCreatePinnedToCore(
    servoTask,
    "ServoTask",
    4096,
    NULL,
    1,
    &servoTaskHandle,
    1
  );

  // servo task on core 1 too
  xTaskCreatePinnedToCore(
    ledTask,
    "LEDTask",
    2048,
    NULL,
    1,
    &ledTaskHandle,
    1
  );
  
  Serial.println("Initialized both APDS and Servo on separate cores.");
  Serial.println("\n=== CONTROL MODES ===");
  Serial.println("DIRECT MODE: White LED - Swipe gestures control arm");
  Serial.println("Cover both sensors: Enter servo selection mode");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(100));
}

// Task 1: reading gestures (core 0)
void gestureTask(void *parameter) {
  static unsigned long lastStateChange = 0;
  const unsigned long STATE_CHANGE_DEBOUNCE = 1000; // 1 second between state changes
  
  while (true) {
    int left_gesture = DIR_NONE;
    int right_gesture = DIR_NONE;
    
    // read gestures from sensors
    if (left_apds.isGestureAvailable()) {
      left_gesture = readGestureNonBlocking(left_apds);
      
      // trigger state change on NEAR or FAR, with debounce
      if ((left_gesture == DIR_NEAR || left_gesture == DIR_FAR) && 
          (millis() - lastStateChange > STATE_CHANGE_DEBOUNCE)
        ) {
        Serial.println(">>> LEFT: NEAR/FAR detected - changing state <<<");
        advanceControlState();
        lastStateChange = millis();
        left_gesture = DIR_NONE;
      } else if (left_gesture == DIR_NEAR || left_gesture == DIR_FAR) {
        left_gesture = DIR_NONE; // ignore if too soon
      }
      
      if ((left_gesture != DIR_NONE) && (left_gesture != -1)
        ) {
        GestureEvent event = {left_gesture, true};
        xQueueSend(gestureQueue, &event, 0);
        handleGesture(left_gesture, "LEFT");
      }
    }
    
    if (right_apds.isGestureAvailable()) {
      right_gesture = readGestureNonBlocking(right_apds);

      // trigger state change on NEAR or FAR (with debounce)
      if ((right_gesture == DIR_NEAR || right_gesture == DIR_FAR) && 
          (millis() - lastStateChange > STATE_CHANGE_DEBOUNCE)) {
        Serial.println(">>> RIGHT: NEAR/FAR detected - changing state <<<");
        advanceControlState();
        lastStateChange = millis();
        right_gesture = DIR_NONE;
      } else if (right_gesture == DIR_NEAR || right_gesture == DIR_FAR) {
        right_gesture = DIR_NONE;
      }
      
      if (right_gesture != DIR_NONE && right_gesture != -1) {
        GestureEvent event = {right_gesture, false};
        xQueueSend(gestureQueue, &event, 0);
        handleGesture(right_gesture, "RIGHT");
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Task 2: control servos (core 1)
void servoTask(void *parameter) {
  GestureEvent event;
  
  while (true) {
    if (xQueueReceive(gestureQueue, &event, pdMS_TO_TICKS(10)) == pdTRUE) {
      
      // handle gesture based on current state
      switch (control_state) {
        case STATE_DIRECT:
          handleDirectGesture(event);
          break;
          
        case STATE_SELECT_SERVO:
          handleSelectionGesture(event.gesture);
          break;
          
        case STATE_ADJUST_SERVO:
          handleAdjustGesture(event.gesture);
          break;
      }

      // flush queue
      int flushed = 0;
      while (xQueueReceive(gestureQueue, &event, 0) == pdTRUE) {
        flushed++;
      }
      if (flushed > 0) {
        Serial.printf("Flushed %d queued gestures\n", flushed);
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Task 2: control LEDS (core 1)
void ledTask(void *parameter) {
  const unsigned long BLINK_INTERVAL = 500; // 500ms on/off
  bool ledState = false;
  unsigned long lastBlink = 0;
  
  while (true) {
    unsigned long now = millis();
    switch (control_state) {
      case STATE_DIRECT:
        // Solid white
        setRGBColor(COLOR_WHITE);
        break;
        
      case STATE_SELECT_SERVO:
        // Blink selected servo color
        if (now - lastBlink >= BLINK_INTERVAL) {
          ledState = !ledState;
          lastBlink = now;
        }
        
        if (ledState && selected_servo_index >= 0 && selected_servo_index < SERVO_COUNT) {
          setRGBColor(servoColors[selected_servo_index]);
        } else {
          setRGBColorPWM(0, 0, 0); // Off
        }
        break;
        
      case STATE_ADJUST_SERVO:
        // Solid color for selected servo
        if (selected_servo_index >= 0 && selected_servo_index < SERVO_COUNT) {
          setRGBColor(servoColors[selected_servo_index]);
        }
        break;
    }
    
    vTaskDelay(pdMS_TO_TICKS(50)); // Check every 50ms
  }
}

void ledInitialization() {
  ledcSetup(6, 5000, 8); // channel 6, 5kHz, 8-bit resolution
  ledcSetup(7, 5000, 8); // channel 7
  ledcSetup(8, 5000, 8); // channel 8
  
  ledcAttachPin(LED_PIN_RED, 6);
  ledcAttachPin(LED_PIN_GREEN, 7);
  ledcAttachPin(LED_PIN_BLUE, 8);
  
  // initial color
  setRGBColor(COLOR_WHITE);
  
  Serial.println("RGB LED initialized");
}

void setRGBColor(RGBColor color) {
  setRGBColorPWM(color.r, color.g, color.b);
}

void setRGBColorPWM(uint8_t r, uint8_t g, uint8_t b) {
  ledcWrite(6, (uint8_t)(r * LED_BRIGHTNESS)); // red
  ledcWrite(7, (uint8_t)(g * LED_BRIGHTNESS)); // green
  ledcWrite(8, (uint8_t)(b * LED_BRIGHTNESS)); // blue
}

void advanceControlState() {
  // clear gesture queue when changing states
  xQueueReset(gestureQueue);
  
  switch (control_state) {
    case STATE_DIRECT:
      control_state = STATE_SELECT_SERVO;
      selected_servo_index = 0;
      Serial.println("\n========================================");
      Serial.println("MODE: SERVO SELECTION");
      Serial.println("Swipe UP/DOWN to select servo");
      Serial.println("========================================");
      announceSelectedServo();
      break;
      
    case STATE_SELECT_SERVO:
      control_state = STATE_ADJUST_SERVO;
      Serial.println("\n========================================");
      Serial.printf("MODE: ADJUSTING %s SERVO\n", servoLabels[selected_servo_index]);
      Serial.println("Swipe UP/DOWN to move servo");
      Serial.println("========================================");
      break;
      
    case STATE_ADJUST_SERVO:
    default:
      control_state = STATE_DIRECT;
      selected_servo_index = -1;
      Serial.println("\n========================================");
      Serial.println("MODE: DIRECT CONTROL");
      Serial.println("Gestures control the arm");
      Serial.println("========================================");
      break;
  }
}

void announceSelectedServo() {
  if (selected_servo_index < 0 || selected_servo_index >= SERVO_COUNT) return;
  
  Serial.printf(">>> Selected: [%d] %s @ %d° <<<\n",
                selected_servo_index,
                servoLabels[selected_servo_index],
                servoRefs[selected_servo_index]->get_current_angle());
}

void handleDirectGesture(const GestureEvent& event) {
  if (!event.isLeft) { // right sensor
    if (event.gesture == DIR_LEFT) {
      moveHorizontalArm("UP");
    } else if (event.gesture == DIR_RIGHT) {
      moveHorizontalArm("DOWN");
    }
  } else { // left sensor
    if (event.gesture == DIR_LEFT) {
      moveHorizontalArm("DOWN");
    } else if (event.gesture == DIR_RIGHT) {
      moveHorizontalArm("UP");
    }
  }
}

void handleSelectionGesture(int gesture) {
  if (gesture == DIR_UP) {
    selected_servo_index = (selected_servo_index - 1 + SERVO_COUNT) % SERVO_COUNT;
    announceSelectedServo();
  } else if (gesture == DIR_DOWN) {
    selected_servo_index = (selected_servo_index + 1) % SERVO_COUNT;
    announceSelectedServo();
  }
}

void handleAdjustGesture(int gesture) {
  if (selected_servo_index < 0 || selected_servo_index >= SERVO_COUNT) return;
  
  ServoController* servo = servoRefs[selected_servo_index];
  int current = servo->get_current_angle();

  if (gesture == DIR_UP) {
    servo->safe_servo_write(current + SERVO_STEP_DEGREES);
    Serial.printf("%s: %d° -> %d°\n",
                  servoLabels[selected_servo_index],
                  current,
                  servo->get_current_angle());
  } else if (gesture == DIR_DOWN) {
    servo->safe_servo_write(current - SERVO_STEP_DEGREES);
    Serial.printf("%s: %d° -> %d°\n",
                  servoLabels[selected_servo_index],
                  current,
                  servo->get_current_angle());
  }
}

void apdsInitialization() {
  I2C_left.begin(LEFT_SDA_PIN, LEFT_SCL_PIN, 100000);
  I2C_right.begin(RIGHT_SDA_PIN, RIGHT_SCL_PIN, 100000);
  
  I2C_left.setTimeout(50);
  I2C_right.setTimeout(50);

  if (left_apds.init()) Serial.println("Left sensor initialized");
  else Serial.println("Left sensor failed");

  if (right_apds.init()) Serial.println("Right sensor initialized");
  else Serial.println("Right sensor failed");

  left_apds.setGestureGain(GGAIN_4X);
  left_apds.setGestureLEDDrive(LED_DRIVE_50MA);
  left_apds.setProximityGain(PGAIN_4X);
  left_apds.setGestureEnterThresh(40);
  left_apds.setGestureExitThresh(30);

  right_apds.setGestureGain(GGAIN_4X);
  right_apds.setGestureLEDDrive(LED_DRIVE_50MA);
  right_apds.setProximityGain(PGAIN_4X);
  right_apds.setGestureEnterThresh(40);
  right_apds.setGestureExitThresh(30);

  if (left_apds.enableProximitySensor(false)) Serial.println("Left proximity enabled");
  else Serial.println("Left proximity failed");

  if (right_apds.enableProximitySensor(false)) Serial.println("Right proximity enabled");
  else Serial.println("Right proximity failed");

  if (left_apds.enableGestureSensor(true)) Serial.println("Left gesture enabled");
  else Serial.println("Left gesture failed");

  if (right_apds.enableGestureSensor(true)) Serial.println("Right gesture enabled");
  else Serial.println("Right gesture failed");
}

void handleGesture(int gesture, const char* sensor_name) {
  if (gesture == DIR_NEAR || gesture == DIR_FAR) {
    return;
  }
  
  Serial.print(sensor_name);
  Serial.print(": ");
  
  switch (gesture) {
    case DIR_UP:
      Serial.println("UP");
      break;
    case DIR_DOWN:
      Serial.println("DOWN");
      break;
    case DIR_LEFT:
      Serial.println("LEFT");
      break;
    case DIR_RIGHT:
      Serial.println("RIGHT");
      break;
    default:
      Serial.println("NONE");
  }
}

int readGestureNonBlocking(SparkFun_APDS9960& apds) {
  int gesture = apds.readGesture();
  return (gesture == -1 || gesture == 0) ? DIR_NONE : gesture;
}

void servoInitialization() {
  servo_base.attach(SIGNAL_PIN_SERVO_BASE, 1, true, 0, {20, 90});
  servo_middle.attach(SIGNAL_PIN_SERVO_MIDDLE, 2, true, 0, {0, 90});
  servo_cross.attach(SIGNAL_PIN_SERVO_CROSS, 3, true, 0, {0, 180});
  servo_left.attach(SIGNAL_PIN_SERVO_LEFT, 4, true, 0, {0, 180});
  servo_right.attach(SIGNAL_PIN_SERVO_RIGHT, 5, true, 180, {0, 180});
}

void moveHorizontalArm(String direction) {
  if (direction == "UP") {
    servo_base.move_to("sin", 20);
    servo_middle.move_to("sin", 0);
    servo_cross.move_to("sin", 0);
    servo_left.move_to("sin", 0);
    servo_right.move_to("sin", 0);
  }

  if (direction == "DOWN") {
    servo_base.move_to("cos", 90);
    servo_middle.move_to("cos", 90);
    servo_cross.move_to("cos", 180);
    servo_left.move_to("cos", 180);
    servo_right.move_to("cos", 180);
  }
}