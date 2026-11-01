# Nugget Self Balancing Desk Buddy

A 2-wheeled self-balancing companion robot powered by an ESP32-S3 DevKitC-1 microcontroller. This project aims to demonstrate capacitive touch sensors, digital I2S audio feedback, real-time MPU6050 PID balancing, a neck servo mechanism and an on-board, custom 1.8" TFT display with an embedded web dashboard via the web.

<img width="500" alt="image" src="https://github.com/user-attachments/assets/a2a8372d-382e-431e-a9a8-776102be1087" />
<img width="500" alt="image" src="https://github.com/user-attachments/assets/da833e06-0b3b-438f-8dcd-0f77fca0d122" />

# Why I made this

Most desktop gadgets and robots sit static or require heavy cloud infrastructure and external routers to function. I wanted to design an interactive, self-contained hardware companion that pushes the limits of desktop robotics by combining:

  - **Real-Time Physics & Control:** Challenging myself to write a high-speed (~200Hz) non-zero PID feedback loop that fuses MPU6050 sensor data to activity defy gravity.
  - **Tactile & Expressive Personality:** Moving beyond a standard utilitarian rover by giving  the robot life through capacitive touch sensors, an I2S audio speaker system,       and a 1-plane neck servo mechanism.
  - **Completely Autonomous IoT:** Building a lightweight, embedded HTTP web server and Wi-Fi Access Point so the robot can be teleoperated instantly from any smartphone
    browser without needing internet or external routers.

# Features
- **Real-Time Self-Balancing:** Uses an MPU6050 6-axis IMU fused with a software complementary filter and a PID control loop to keep the robot upright.
- **Multimodal State Machine:** Features 5 distinct operational modes (Self-Balance, Weather Station, RC Drive, Pomodoro Timer, and Sleep Mode).
- **Physical Touch Interaction:** Two capacitive touch sensors (one on the head, and one on the stomach to cycle modes).
- **Neck Servo Mechanism:** A 1-plane micro servo allows the robot to look around, nod, and express personality.
- **Digital Audio Output:** Uses a MAX98357A I2S amplifier and micro speaker to play dynamic tones, chimes, and alarms.
- **Onboard Web Dashboard:** Broadcasts a local Wi-Fi Access Point (Robot-Companion) allowing full teleoperation and telemetry from any smartphone browser without external routers.
- **1.8" TFT Visual UI:** Renders system states, mode indicators, and expression layouts in real-time.

# How It Works

The ESP32-S3 is the main controller with Arduino C++ firmware. It runs high speed non blocking loops (~200Hz) that integrates IMU sensor fusion, motor PWM outputs, capacitive touch polling and an HTTP server.

Balance Loop: The ESP32-S3 receives accelerometer and gyroscope data from the MPU6050 using I2C. First the complementary filter filters out the noise in the signal, and the PID controller controls the N20 motors though the L298N Mini driver.

Touch & Servo Controls: Switch between modes of operation thorough a state machine when the sensor on the stomach is tapped. Pressing the head sensors causes a nodding of the head and a chirp sound.

Web Teleoperation: If the Access Point is connected to the robots local network, visit 192.168.4.1 to control the robot's drive direction, the neck servo, push weather data, or switch modes remotely.

# Hardware Overview

| Component | Purpose / Function |
| :--- | :--- |
| **ESP32-S3 DevKitC-1** | Main microcontroller core with dual-core processing and built-in Wi-Fi |
| **MPU6050 Module** | 6-axis inertial measurement unit for tilt and balance detection |
| **1.8" SPI TFT Display** | Renders operational status, menus, and visual feedback |
| **MAX98357A Amplifier** | Converts I²S digital audio streams into amplified speaker output |
| **Micro Speaker** | Plays system chimes, button clicks, and timer alarms |
| **L298N Mini Driver** | Dual H-bridge motor driver supplying PWM speed control to motors |
| **2x N20 Geared DC Motors** | Drive motors providing high-torque wheel actuation |
| **SG90 Micro Servo** | Controls 1-plane mechanical neck movement |
| **2x Capacitive Touch Pads** | Acts as interactive inputs for the head and stomach |
| **2S 18650 Battery Pack** | Provides ~7.4V primary system power |
| **DC-DC Step-Down Buck** | Steps 7.4V battery voltage down to a clean regulated 5V rail |


# Wiring Diagram & Pin Connections

<img width="733" height="398" alt="image" src="https://github.com/user-attachments/assets/4e4255d1-422c-454d-807e-421793afcee2" />

| Module / Signal | ESP32-S3 Pin |
| :--- | :--- |
| **MPU6050 SDA** | `GPIO 8` |
| **MPU6050 SCL** | `GPIO 9` |
| **TFT SCK** | `GPIO 7` |
| **TFT MOSI (SDA)** | `GPIO 11` |
| **TFT CS** | `GPIO 10` |
| **TFT RESET** | `GPIO 12` |
| **TFT DC (A0)** | `GPIO 13` |
| **MAX98357A DIN** | `GPIO 15` |
| **MAX98357A BCLK** | `GPIO 16` |
| **MAX98357A LRC** | `GPIO 17` |
| **Neck Servo Signal** | `GPIO 4` |
| **Head Touch Sensor** | `GPIO 1` |
| **Stomach Touch Sensor** | `GPIO 2` |
| **L298N IN1 / IN2** | `GPIO 5 / GPIO 6` |
| **L298N IN3 / IN4** | `GPIO 14 / GPIO 38` |

# Software Setup
The firmware is written in Arduino C++ for the ESP32-S3.

Required Libraries
Adafruit GFX Library

Adafruit ST7735 and ST7789 Library

ESP32Servo

Built-in board core libraries (WiFi, WebServer, Wire, driver/i2s.h)

Setup Instructions
Install the Arduino IDE and configure the ESP32 board package.

Install the required Adafruit display and servo libraries via the Arduino Library Manager.

Wire all electronic modules according to the pin table and power schematic.

Open firmware/Robot_Professional_Sleep.ino in the Arduino IDE.

Select your ESP32-S3 board and correct COM port, then flash the firmware.

Connect your phone or computer to the Wi-Fi network Robot-Companion (Password: nugget1234) and navigate to [http://192.168.4.1](http://192.168.4.1)

| Item | Qty | Supplier | Cost/item | Total Cost | Link |
| :--- | :---: | :--- | :---: | :---: | :--- |
| **ESP32-S3-DevKitC-1** | 1 | AliExpress | $6.36 | $6.36 | [View Link](https://www.aliexpress.com/item/1005006737960797.html) |
| **1.8" TFT Display** | 1 | AliExpress | $3.47 | $3.47 | [View Link](https://www.aliexpress.com/item/1005006139989470.html) |
| **Capacitive Touch Sensors** | 1 | AliExpress | $2.96 | $2.96 | [View Link](https://www.aliexpress.com/item/1005006246380749.html) |
| **MAX98357A Audio Amplifier** | 1 | AliExpress | $1.55 | $1.55 | [View Link](https://www.aliexpress.com/item/1005007003802663.html) |
| **L298N Mini** | 1 | AliExpress | $1.57 | $1.57 | [View Link](https://www.aliexpress.com/item/1005012607171084.html) |
| **18650 Lithium Cells (2-Pack)** | 1 | Lord Of Robots | $7.16 | $7.16 | [View Link](https://lordofrobots.com/products/18650b-lithium-ion-battery-2-pcs) |
| **18650 Battery Holder** | 1 | Lord Of Robots | $2.68 | $2.68 | [View Link](https://lordofrobots.com/products/battery-holder-18650) |
| **MPU6050** | 1 | AliExpress | $2.43 | $2.43 | [View Link](https://www.aliexpress.com/item/1005010057794277.html) |
| **Estimated total shipping** | 1 | -- | $0.00 | $0.00 | -- |
| **Total Estimated Cost** | | | | **~$28.18 CAD** | |




