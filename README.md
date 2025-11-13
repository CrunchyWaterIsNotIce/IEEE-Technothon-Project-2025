# Gesture-Grip (IEEE-Technothon Project 2025)

## Project overview
  The Gesture-Grip allows for **touchless control and seamlessly integrates itself into the user's workflow**. Instead of interrupting work to manually adjust each arm and position of a suboptimal helping hand, the user can simply swipe towards the Gesture-Grip which provides **precise, stable, and hands-free positioning** for delicate components, wires, and PCBs, letting the user keep their tools in hand for a faster, more efficient, and less frustrating workflow. There are **3 settings built into the Gesture-Grip** and each of the setting states are indicated by a singular RGB led housed on the base.
  
## Hardware components used
- 1x [ESP-WROOM-32](https://www.amazon.com/dp/B08D5ZD528?ref=ppx_yo2ov_dt_b_fed_asin_title&th=1)
- 2x [APDS-9960 Gesture Sensor Modules](https://www.amazon.com/dp/B01NACU412?ref=ppx_yo2ov_dt_b_fed_asin_title)
- 5x [SG90 Micro Servo Motors](https://www.amazon.com/dp/B07NSVKZP7?ref=ppx_yo2ov_dt_b_fed_asin_title) 
- [Breadboarding Materials](https://www.amazon.com/dp/B073ZC68QG?ref=ppx_yo2ov_dt_b_fed_asin_title)
- [External Power Supply](https://www.amazon.com/dp/B08BL4QMGM?ref=ppx_yo2ov_dt_b_fed_asin_title)
- [3D Printed Components of Arm Structure](https://github.com/CrunchyWaterIsNotIce/IEEE-Technothon-Project-2025/tree/main/3D_Models)
## Libraries, APIs, and frameworks used

| **Name** | **Purpose** |
| :--- | :--- |
| **PlatformIO** | Cross-platform build system used with the Arduino IDE |
| **ESP32Servo** | Library for driving the SG90 servo motors efficiently on the ESP32 |
| **SparkFun_APDS9960** | Library for reading data from the gesture sensors |
| **FreeRTOS** | Manages parallel functions between servo movement and gesture reading |
| **Solidworks** | Used for 3D modeling the custom arm and base components |

## Setup instructions
1. **POWER ON**
  -   Connect the External Power Supply to the servo motor rail.
  -   Connect the ESP-WROOM-32 via its power source (e.g., USB).

      The RGB LED on the base will illuminate, indicating the initial control state (Presets).
      
***

2. **STATE CONTROL (Switching Modes)**
- The Gesture-Grip has two main operating modes (Presets $\rightarrow$ Selecting/Moving Servo Joint):
  
- **Cycling Through States** $\rightarrow$ (The LED should be blinking) **Swipe your hand left/ right** in front of the LEFT sensor.
  
- **Selecting a State** $\rightarrow$ Place your **finger in close proximity** to the RIGHT gesture sensor then **swiftly pull your hand back**. The LED should stop blinking and become a static color indicating the selection chosen.

***

3. **SELECTING AND MOVING A JOINT**
- **Select Joint** $\rightarrow$ When in the Selecting Servo Joint state (Blinking LED), swipe left or right in front of the LEFT sensor to find the servo you want to control (indicated by the blinking color).

- **Confirm Selection** $\rightarrow$ To lock onto that joint, hold your finger stationary in front of RIGHT sensor, then perform the quick pull back gesture.

    The LED will stop blinking and become a static color, putting you in the Moving Servo Joint state.

- **Move Joint** $\rightarrow$ Swipe left or right in the front of the LEFT sensor to make precise angular adjustments to the selected joint.

    To switch back to another mode, perform the quick pull back towards the RIGHT sensor again (The LED should start blinking again).

***

| **State (LED Indicator)** | **Gesture** | **Function** |
| :--- | :--- | :--- |
| Presets (Static White Light) |	UP / DOWN	| Cycles through pre-programmed arm positions. |
| Selecting Servo Joint (Blinking Respective Color)	| LEFT / RIGHT	| Swipes through the 5 individual servo joints (each associated with a unique blinking color). |
| Moving Servo Joint (Static Respective Color)	| LEFT / RIGHT	| Adjusts the angle of the currently selected servo joint. |

## Team members and roles

- @ChristinChi:
  - Contributed to the initial design by drafting concept sketches and carefully measuring components. The goal was to create a solid foundation for the CAD model and help ensure all parts would function together smoothly.
  - Handled all the 3D modeling for the 'Gesture-Grip' in Solidworks, accurately translating the team's concept sketches into a detailed, multi-part assembly ready for fabrication.
  - Led the physical assembly and iterative testing, identifying and resolving mechanical fit issues, and refining the CAD models to improve the arm's stability and range of motion.
  - Maintained the project's documentation on Github and Devpost by clearly explaining the project's goals and outcomes.
- @CrunchyWaterIsNotIce:
  - Spearheaded the direction of the overall lead of the project.
  - Initiated the whole software and hardware design of 'Gesture-Grip' through coordinated research and deployment of knowledge using the libraries and frameworks listed above.
  - Architected the complete software state machine, using FreeRTOS to manage concurrent tasks for real-time gesture reading and smooth, precise servo control, ensuring the system was responsive and intuitive.
  - Strategically selected hardware and software that met real-time performance goals and fit within a manageable implementation timeline, which prevented over-complication and allowed for thorough work.
- Chiikabu:
  - Looked cute.
