/**
 * @file sensors.h
 * @brief Sensor abstraction layer
 * @details Provides clean interface to all robot sensors
 */

#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include "../config/pins.h"
#include "../config/constants.h"

// ============================================================================
// COLOR ENUMERATION
// ============================================================================
enum Color : uint8_t {
  COLOR_NONE = 0,
  COLOR_BLACK,
  COLOR_WHITE,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE
};

// ============================================================================
// SENSOR DATA STRUCTURE
// ============================================================================
struct SensorData {
  // Color sensor
  uint16_t redFreq;
  uint16_t greenFreq;
  uint16_t blueFreq;
  Color detectedColor;
  Color previousColor;
  
  // Ultrasonic
  float distance;
  bool obstacleDetected;
  
  // IR sensors
  bool leftOnLine;
  bool rightOnLine;
};

// ============================================================================
// SENSOR CLASS
// ============================================================================
class Sensors {
public:
  /**
   * @brief Initialize all sensors
   * @note Must be called in setup()
   */
  static void init();
  
  /**
   * @brief Read all sensors and update data structure
   * @param data Reference to SensorData structure to populate
   * @note O(1) time complexity - fixed number of operations
   */
  static void readAll(SensorData& data);
  
  /**
   * @brief Read ultrasonic sensor only
   * @return Distance in centimeters (0-400, 999 on timeout)
   * @note O(1) - single measurement
   */
  static float readDistance();
  
  /**
   * @brief Read color sensor frequencies
   * @param r Reference to store red frequency
   * @param g Reference to store green frequency  
   * @param b Reference to store blue frequency
   * @note O(1) - three fixed measurements
   */
  static void readColorRaw(uint16_t& r, uint16_t& g, uint16_t& b);
  
  /**
   * @brief Detect color from frequency values
   * @param r Red frequency
   * @param g Green frequency
   * @param b Blue frequency
   * @return Detected Color enum value
   * @note O(1) - fixed comparisons
   */
  static Color detectColor(uint16_t r, uint16_t g, uint16_t b);
  
  /**
   * @brief Read IR sensors
   * @param leftOnLine Reference to store left sensor state
   * @param rightOnLine Reference to store right sensor state
   * @note O(1) - two digital reads
   */
  static void readIR(bool& leftOnLine, bool& rightOnLine);
  
  /**
   * @brief Get color name as string (for debugging)
   * @param c Color enum value
   * @return Pointer to color name string
   */
  static const char* colorName(Color c);

private:
  static void readUltrasonic(SensorData& data);
  static void readColor(SensorData& data);
};

#endif // SENSORS_H
