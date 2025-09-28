// Rocket Flight Computer - Arduino Nano (ATmega328P) Version
// Features: MPU6050 IMU, BMP280 Barometer, SD Card Logging, Dual Servo TVC Control

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <SD.h>
#include <Servo.h>
#include "I2Cdev.h"
#include "MPU6050.h"

// Sensor and peripheral objects
MPU6050 mpu;
Adafruit_BMP280 bmp;
Servo servoX, servoY;

// Sensor values
int16_t ax, ay, az, gx, gy, gz;
float altitude = 0;

// SD card
const int chipSelect = 10;
File dataFile;
unsigned long lastLog = 0;
const int logInterval = 50; // 50ms = 20Hz

// Servo pins
const int SERVO_X_PIN = 5;
const int SERVO_Y_PIN = 6;

// LED indicator
const int LED_PIN = 13;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  Wire.begin();

  // Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);
  }

  // Initialize BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 not found");
    while (1);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_1);

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card failed");
    while (1);
  }
  dataFile = SD.open("flight.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Time,Ax,Ay,Az,Gx,Gy,Gz,Altitude");
    dataFile.close();
  }

  // Attach servos
  servoX.attach(SERVO_X_PIN);
  servoY.attach(SERVO_Y_PIN);
  servoX.write(90);
  servoY.write(90);
}

void loop() {
  unsigned long now = millis();

  // Read IMU data
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Read altitude
  altitude = bmp.readAltitude(1013.25); // Sea level pressure in hPa

  // Control servos (placeholder)
  int controlX = map(gx, -25000, 25000, 60, 120);
  int controlY = map(gy, -25000, 25000, 60, 120);
  servoX.write(constrain(controlX, 60, 120));
  servoY.write(constrain(controlY, 60, 120));

  // Log to SD card
  if (now - lastLog >= logInterval) {
    dataFile = SD.open("flight.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.print(now);
      dataFile.print(","); dataFile.print(ax);
      dataFile.print(","); dataFile.print(ay);
      dataFile.print(","); dataFile.print(az);
      dataFile.print(","); dataFile.print(gx);
      dataFile.print(","); dataFile.print(gy);
      dataFile.print(","); dataFile.print(gz);
      dataFile.print(","); dataFile.println(altitude);
      dataFile.close();
    }
    lastLog = now;
  }

  delay(5); // Short delay to reduce I2C bus contention
}
