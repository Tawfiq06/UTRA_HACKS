/**
 * @file sensors.cpp
 * @brief Sensor abstraction layer implementation
 */

#include "sensors.h"

// ============================================================================
// INITIALIZATION
// ============================================================================
void Sensors::init() {
  // Color sensor pins
  pinMode(PIN_COLOR_S0, OUTPUT);
  pinMode(PIN_COLOR_S1, OUTPUT);
  pinMode(PIN_COLOR_S2, OUTPUT);
  pinMode(PIN_COLOR_S3, OUTPUT);
  pinMode(PIN_COLOR_OUT, INPUT);
  
  // Set color sensor frequency scaling to 20%
  // This provides optimal balance of speed and accuracy
  digitalWrite(PIN_COLOR_S0, HIGH);
  digitalWrite(PIN_COLOR_S1, LOW);
  
  // Ultrasonic pins
  pinMode(PIN_ULTRA_TRIG, OUTPUT);
  pinMode(PIN_ULTRA_ECHO, INPUT);
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  
  // IR sensor pins
  pinMode(PIN_IR_LEFT, INPUT);
  pinMode(PIN_IR_RIGHT, INPUT);
}

// ============================================================================
// READ ALL SENSORS - O(1)
// ============================================================================
void Sensors::readAll(SensorData& data) {
  // Store previous color for transition detection
  data.previousColor = data.detectedColor;
  
  // Read each sensor subsystem
  readUltrasonic(data);
  readColor(data);
  readIR(data.leftOnLine, data.rightOnLine);
}

// ============================================================================
// ULTRASONIC SENSOR - O(1)
// ============================================================================
void Sensors::readUltrasonic(SensorData& data) {
  data.distance = readDistance();
  data.obstacleDetected = (data.distance > 0 && data.distance < DIST_OBSTACLE);
}

float Sensors::readDistance() {
  // Generate 10µs trigger pulse
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRA_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  
  // Measure echo pulse duration
  const unsigned long duration = pulseIn(PIN_ULTRA_ECHO, HIGH, ULTRA_TIMEOUT_US);
  
  // Convert to distance (speed of sound = 343m/s = 0.034cm/µs)
  // Divide by 2 for round trip
  if (duration == 0) {
    return 999.0f;  // Timeout - no obstacle detected
  }
  
  return (duration * ULTRA_SPEED_CM) / 2.0f;
}

// ============================================================================
// COLOR SENSOR - O(1)
// ============================================================================
void Sensors::readColor(SensorData& data) {
  readColorRaw(data.redFreq, data.greenFreq, data.blueFreq);
  data.detectedColor = detectColor(data.redFreq, data.greenFreq, data.blueFreq);
}

void Sensors::readColorRaw(uint16_t& r, uint16_t& g, uint16_t& b) {
  // Read RED photodiodes (S2=L, S3=L)
  digitalWrite(PIN_COLOR_S2, LOW);
  digitalWrite(PIN_COLOR_S3, LOW);
  delay(TIME_SENSOR_READ);
  r = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  // Read GREEN photodiodes (S2=H, S3=H)
  digitalWrite(PIN_COLOR_S2, HIGH);
  digitalWrite(PIN_COLOR_S3, HIGH);
  delay(TIME_SENSOR_READ);
  g = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  // Read BLUE photodiodes (S2=L, S3=H)
  digitalWrite(PIN_COLOR_S2, LOW);
  digitalWrite(PIN_COLOR_S3, HIGH);
  delay(TIME_SENSOR_READ);
  b = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  // Handle timeout (0 = no reading)
  if (r == 0) r = 999;
  if (g == 0) g = 999;
  if (b == 0) b = 999;
}

Color Sensors::detectColor(uint16_t r, uint16_t g, uint16_t b) {
  // BLACK: All colors reflect poorly (high frequency = low reflectance)
  if (r > COLOR_FREQ_BLACK && g > COLOR_FREQ_BLACK && b > COLOR_FREQ_BLACK) {
    return COLOR_BLACK;
  }
  
  // WHITE: All colors reflect strongly (low frequency = high reflectance)
  if (r < COLOR_FREQ_WHITE && g < COLOR_FREQ_WHITE && b < COLOR_FREQ_WHITE) {
    return COLOR_WHITE;
  }
  
  // RED: Red frequency significantly lower than others
  if (r < g - COLOR_MARGIN && r < b - COLOR_MARGIN && r < COLOR_FREQ_MAX) {
    return COLOR_RED;
  }
  
  // GREEN: Green frequency significantly lower than others
  if (g < r - COLOR_MARGIN && g < b - COLOR_MARGIN && g < COLOR_FREQ_MAX) {
    return COLOR_GREEN;
  }
  
  // BLUE: Blue frequency significantly lower than others
  if (b < r - COLOR_MARGIN && b < g - COLOR_MARGIN && b < COLOR_FREQ_MAX) {
    return COLOR_BLUE;
  }
  
  return COLOR_NONE;
}

// ============================================================================
// IR SENSORS - O(1)
// ============================================================================
void Sensors::readIR(bool& leftOnLine, bool& rightOnLine) {
  // IR sensors return LOW when detecting black line
  leftOnLine = (digitalRead(PIN_IR_LEFT) == IR_ON_LINE);
  rightOnLine = (digitalRead(PIN_IR_RIGHT) == IR_ON_LINE);
}

// ============================================================================
// UTILITY - O(1)
// ============================================================================
const char* Sensors::colorName(Color c) {
  // Use PROGMEM-style lookup for memory efficiency
  static const char* const names[] = {
    "NONE", "BLACK", "WHITE", "RED", "GREEN", "BLUE"
  };
  return names[c];
}
