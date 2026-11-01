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
- **Onboard Web Dashboard:** Broadcasts a local Wi-Fi Access Point
