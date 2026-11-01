#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <driver/i2s.h>

// ==================== PIN DEFINITIONS ====================
// MPU6050 I2C Pins
#define I2C_SDA 8
#define I2C_SCL 9

// 1.8" TFT Display (SPI) Pins
#define TFT_CS    10
#define TFT_RST   12
#define TFT_DC    13
#define TFT_MOSI  11
#define TFT_SCK   7

// L298N Mini Motor Driver Pins
#define MOTOR_IN1 5
#define MOTOR_IN2 6
#define MOTOR_IN3 14
#define MOTOR_IN4 38

// Neck Servo Pin
#define SERVO_PIN 4

// Capacitive Touch Pins
#define TOUCH_HEAD    1
#define TOUCH_STOMACH 2

// I2S Audio Pins (MAX98357A)
#define I2S_DIN   15
#define I2S_BCLK  16
#define I2S_LRC   17

// ==================== OBJECTS & VARIABLES ====================
WebServer server(80);
Servo neckServo;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

// Robot States
enum RobotMode { MODE_BALANCE = 1, MODE_WEATHER = 2, MODE_RC_DRIVE = 3, MODE_TIMER = 4 };
int currentMode = MODE_BALANCE;
int neckAngle = 90;
String driveCmd = "stop";

// MPU6050 & PID Variables
const int mpuAddress = 0x68;
float gyroY, accelAngleY, currentAngle = 0.0, targetAngle = 0.0;
float Kp = 22.5, Kd = 1.8; // PID Tuning Constants
unsigned long lastTime = 0;
float dt = 0.01;

// Timer Mode Variables
unsigned long timerStartTime = 0;
unsigned long timerDuration = 25 * 60 * 1000; // 25 minutes (Pomodoro)
bool timerActive = false;

// Touch Debounce
unsigned long lastStomachTouch = 0;
unsigned long lastHeadTouch = 0;
const unsigned long touchCooldown = 400;

// Weather string cache
String currentWeatherTemp = "--°C";
String currentWeatherDesc = "Waiting for data...";

// ==================== HTML DASHBOARD UI ====================
const char MAIN_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Robot Command Center</title>
  <style>
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #121212; color: #fff; text-align: center; margin: 0; padding: 15px; }
    h1 { color: #00e676; font-size: 22px; margin-bottom: 5px; }
    .card { background: #1e1e1e; border-radius: 12px; padding: 15px; margin-bottom: 15px; box-shadow: 0 4px 6px rgba(0,0,0,0.4); }
    .btn { background: #00e676; color: #000; border: none; padding: 12px; font-size: 18px; margin: 4px; border-radius: 8px; font-weight: bold; width: 75px; height: 50px; cursor: pointer; }
    .btn:active { background: #00a854; }
    .btn-mode { width: 45%; background: #29b6f6; color: #000; height: auto; padding: 12px; margin: 4px; font-size: 15px; }
    .btn-mode:active { background: #0288d1; }
    input[type=range] { width: 100%; margin: 15px 0; accent-color: #00e676; }
    .weather-box { font-size: 18px; color: #ffeb3b; font-weight: bold; }
  </style>
</head>
<body>
  <h1>🤖 Robot Command Center</h1>
  <p style="color:#aaa; font-size:13px;">ESP32-S3 Companion Core</p>

  <!-- Mode Switcher -->
  <div class="card">
    <h3>Operational Mode</h3>
    <button class="btn btn-mode" onclick="setMode(1)">⚖️ Balance</button>
    <button class="btn btn-mode" onclick="setMode(2)">🌤️ Weather</button>
    <button class="btn btn-mode" onclick="setMode(3)">🎮 RC Drive</button>
    <button class="btn btn-mode" onclick="setMode(4)">⏰ Timer</button>
  </div>

  <!-- RC Drive / Teleop -->
  <div class="card" id="driveCard">
    <h3>Manual Drive Pad</h3>
    <button class="btn" onclick="sendCmd('forward')">▲</button><br>
    <button class="btn" onclick="sendCmd('left')">◀</button>
    <button class="btn" onclick="sendCmd('stop')" style="background:#ff5252; color:#fff;">⏹</button>
    <button class="btn" onclick="sendCmd('right')">▶</button><br>
    <button class="btn" onclick="sendCmd('backward')">▼</button>
  </div>

  <!-- Weather Push Card -->
  <div class="card">
    <h3>Weather Dashboard</h3>
    <div class="weather-box" id="weatherDisplay">--°C | Connected</div>
    <input type="text" id="weatherInput" placeholder="Enter Temp & Cond (e.g. 22C Sunny)" style="width:80%; padding:8px; margin-top:10px; border-radius:6px; border:none;">
    <br><button class="btn" style="width:auto; padding:8px 15px; margin-top:10px; font-size:14px;" onclick="pushWeather()">Push to Robot</button>
  </div>

  <!-- Neck Servo -->
  <div class="card">
    <h3>Neck Servo Angle (<span id="servoVal">90</span>°)</h3>
    <input type="range" min="0" max="180" value="90" oninput="updateServo(this.value)">
  </div>

  <script>
    function sendCmd(dir) { fetch('/drive?dir=' + dir); }
    function updateServo(val) {
      document.getElementById('servoVal').innerText = val;
      fetch('/servo?pos=' + val);
    }
    function setMode(m) { fetch('/mode?val=' + m); }
    function pushWeather() {
      let val = document.getElementById('weatherInput').value;
      fetch('/weather?data=' + encodeURIComponent(val))
        .then(res => res.text())
        .then(txt => { document.getElementById('weatherDisplay').innerText = txt; });
    }
  </script>
</body>
</html>
)rawliteral";

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);

  // 1. Initialize I2C & MPU6050
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.beginTransmission(mpuAddress);
  Wire.write(0x6B);
  Wire.write(0); // Wake up MPU6050
  Wire.endTransmission(true);

  // 2. Initialize TFT Display
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.print("Booting Companion...");

  // 3. Initialize Motors & Servo
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  
  neckServo.attach(SERVO_PIN);
  neckServo.write(neckAngle);

  // 4. Initialize I2S Audio
  setupI2SAudio();
  playTone(880, 150); // Startup Beep

  // 5. Initialize Wi-Fi Access Point
  WiFi.softAP("Robot-Companion", "password123");
  Serial.println("AP Started. IP: 192.168.4.1");

  // 6. Setup Web Server Handlers
  server.on("/", []() { server.send_P(200, "text/html", MAIN_PAGE); });
  server.on("/drive", handleDriveRequest);
  server.on("/servo", handleServoRequest);
  server.on("/mode", handleModeRequest);
  server.on("/weather", handleWeatherRequest);
  server.begin();

  updateScreenDisplay();
}

// ==================== MAIN LOOP ====================
void loop() {
  server.handleClient();
  handleTouchSensors();

  // Mode execution loop
  switch (currentMode) {
    case MODE_BALANCE:
      runBalancingAlgorithm();
      break;
    case MODE_WEATHER:
      // Static display handled on update; idle state
      stopMotors();
      break;
    case MODE_RC_DRIVE:
      handleRCDriveMovement();
      break;
    case MODE_TIMER:
      runTimerCountdown();
      break;
  }
}

// ==================== SUBSYSTEM FUNCTIONS ====================

// MPU6050 Complementary Filter & PID Balancing
void runBalancingAlgorithm() {
  unsigned long now = micros();
  dt = (now - lastTime) / 1000000.0;
  if (dt < 0.005) return; // Limit loop rate (~200Hz)
  lastTime = now;

  // Read Accelerometer & Gyro
  Wire.beginTransmission(mpuAddress);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(mpuAddress, 14, true);
  
  int16_t ax = Wire.read() << 8 | Wire.read();
  int16_t ay = Wire.read() << 8 | Wire.read();
  int16_t az = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // Skip temperature
  int16_t gy = Wire.read() << 8 | Wire.read();

  // Calculate angles
  accelAngleY = atan2((float)ax, (float)az) * 180.0 / PI;
  gyroY = (float)gy / 131.0; // Convert to deg/s

  // Complementary Filter: 98% Gyro + 2% Accel
  currentAngle = 0.98 * (currentAngle + gyroY * dt) + 0.02 * accelAngleY;

  // PID Calculation
  float error = currentAngle - targetAngle;
  int motorSpeed = (int)(Kp * error + Kd * gyroY);
  motorSpeed = constrain(motorSpeed, -255, 255);

  driveMotors(motorSpeed);
}

// Motor Drivers
void driveMotors(int pwmVal) {
  if (pwmVal > 0) {
    analogWrite(MOTOR_IN1, pwmVal); analogWrite(MOTOR_IN2, 0);
    analogWrite(MOTOR_IN3, pwmVal); analogWrite(MOTOR_IN4, 0);
  } else if (pwmVal < 0) {
    int rev = abs(pwmVal);
    analogWrite(MOTOR_IN1, 0); analogWrite(MOTOR_IN2, rev);
    analogWrite(MOTOR_IN3, 0); analogWrite(MOTOR_IN4, rev);
  } else {
    stopMotors();
  }
}

void stopMotors() {
  analogWrite(MOTOR_IN1, 0); analogWrite(MOTOR_IN2, 0);
  analogWrite(MOTOR_IN3, 0); analogWrite(MOTOR_IN4, 0);
}

void handleRCDriveMovement() {
  if (driveCmd == "forward") {
    analogWrite(MOTOR_IN1, 180); analogWrite(MOTOR_IN2, 0);
    analogWrite(MOTOR_IN3, 180); analogWrite(MOTOR_IN4, 0);
  } else if (driveCmd == "backward") {
    analogWrite(MOTOR_IN1, 0); analogWrite(MOTOR_IN2, 180);
    analogWrite(MOTOR_IN3, 0); analogWrite(MOTOR_IN4, 180);
  } else if (driveCmd == "left") {
    analogWrite(MOTOR_IN1, 0); analogWrite(MOTOR_IN2, 150);
    analogWrite(MOTOR_IN3, 150); analogWrite(MOTOR_IN4, 0);
  } else if (driveCmd == "right") {
    analogWrite(MOTOR_IN1, 150); analogWrite(MOTOR_IN2, 0);
    analogWrite(MOTOR_IN3, 0); analogWrite(MOTOR_IN4, 150);
  } else {
    stopMotors();
  }
}

// Touch Sensor State Machine
void handleTouchSensors() {
  // Stomach Touch: Cycle Modes
  if (touchRead(TOUCH_STOMACH) < 40 && millis() - lastStomachTouch > touchCooldown) {
    currentMode = (currentMode % 4) + 1;
    playTone(1046, 80); // High click beep
    updateScreenDisplay();
    lastStomachTouch = millis();
  }

  // Head Touch: Action / Neck Nod
  if (touchRead(TOUCH_HEAD) < 40 && millis() - lastHeadTouch > touchCooldown) {
    playTone(523, 100);
    neckServo.write(45);
    delay(150);
    neckServo.write(90);
    lastHeadTouch = millis();
  }
}

// Pomodoro Timer Logic
void runTimerCountdown() {
  if (!timerActive) return;
  unsigned long elapsed = millis() - timerStartTime;
  if (elapsed >= timerDuration) {
    timerActive = false;
    playTone(784, 500); // Alarm sound
    neckServo.write(0);
    delay(300);
    neckServo.write(180);
    delay(300);
    neckServo.write(90);
  }
}

// TFT Screen Refresh Handler
void updateScreenDisplay() {
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(5, 5);
  tft.setTextSize(1);
  tft.setTextColor(ST7735_CYAN);
  tft.print("MODE: ");
  
  switch (currentMode) {
    case MODE_BALANCE:
      tft.println("Self-Balance");
      tft.setCursor(5, 25);
      tft.setTextColor(ST7735_GREEN);
      tft.print("Status: Active");
      break;
    case MODE_WEATHER:
      tft.println("Weather Station");
      tft.setCursor(5, 25);
      tft.setTextColor(ST7735_YELLOW);
      tft.print(currentWeatherTemp);
      tft.setCursor(5, 40);
      tft.print(currentWeatherDesc);
      break;
    case MODE_RC_DRIVE:
      tft.println("RC Teleop Drive");
      break;
    case MODE_TIMER:
      tft.println("Pomodoro Timer");
      tft.setCursor(5, 25);
      tft.setTextColor(ST7735_MAGENTA);
      tft.print("25m Countdown Ready");
      break;
  }
}

// I2S Audio Tone Generator for MAX98357A
void setupI2SAudio() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 512,
    .use_apll = false
  };
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DIN,
    .data_in_num = I2S_I2S_NO_PIN
  };
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void playTone(int freq, int durationMs) {
  int sampleRate = 16000;
  int numSamples = (sampleRate * durationMs) / 1000;
  int16_t *buffer = (int16_t *)malloc(numSamples * sizeof(int16_t));
  if (!buffer) return;

  for (int i = 0; i < numSamples; i++) {
    float t = (float)i / sampleRate;
    buffer[i] = (sin(2 * PI * freq * t) > 0 ? 1 : -1) * 8000; // Square wave tone
  }
  size_t bytesWritten;
  i2s_write(I2S_NUM_0, buffer, numSamples * sizeof(int16_t), &bytesWritten, portMAX_DELAY);
  free(buffer);
}

// Web Server Endpoints
void handleDriveRequest() {
  if (server.hasArg("dir")) {
    driveCmd = server.arg("dir");
    if (currentMode == MODE_BALANCE) {
      if (driveCmd == "forward") targetAngle = -3.5;
      else if (driveCmd == "backward") targetAngle = 3.5;
      else targetAngle = 0.0;
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleServoRequest() {
  if (server.hasArg("pos")) {
    neckAngle = server.arg("pos").toInt();
    neckServo.write(neckAngle);
  }
  server.send(200, "text/plain", "OK");
}

void handleModeRequest() {
  if (server.hasArg("val")) {
    currentMode = server.arg("val").toInt();
    if (currentMode == MODE_TIMER) {
      timerStartTime = millis();
      timerActive = true;
    }
    updateScreenDisplay();
  }
  server.send(200, "text/plain", "OK");
}

void handleWeatherRequest() {
  if (server.hasArg("data")) {
    currentWeatherTemp = server.arg("data");
    currentWeatherDesc = "Updated via Web UI";
    if (currentMode == MODE_WEATHER) updateScreenDisplay();
    server.send(200, "text/plain", currentWeatherTemp);
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}