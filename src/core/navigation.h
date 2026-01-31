/**
 * @file navigation.h
 * @brief High-level navigation behaviors
 * @details Line following, obstacle avoidance, and path planning
 */

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include "sensors.h"
#include "motors.h"
#include "../config/constants.h"

// ============================================================================
// NAVIGATION RESULT ENUMERATION
// ============================================================================
enum NavResult : uint8_t {
  NAV_CONTINUE = 0,   // Keep navigating
  NAV_TARGET_FOUND,   // Reached target color/position
  NAV_OBSTACLE,       // Obstacle detected
  NAV_LOST,           // Lost the line
  NAV_TIMEOUT         // Operation timed out
};

// ============================================================================
// NAVIGATION CLASS
// ============================================================================
class Navigation {
public:
  /**
   * @brief Follow black line using IR sensors
   * @param data Current sensor readings
   * @return Navigation result
   * @note O(1) - simple state-based decision
   */
  static NavResult followBlackLine(const SensorData& data);
  
  /**
   * @brief Follow colored line using color sensor
   * @param data Current sensor readings
   * @param targetColor Color to follow
   * @return Navigation result
   * @note O(1) - simple state-based decision
   */
  static NavResult followColorLine(const SensorData& data, Color targetColor);
  
  /**
   * @brief Navigate toward center of target (concentric rings)
   * @param data Current sensor readings
   * @return Navigation result
   * @note O(1) - search pattern state machine
   */
  static NavResult navigateToCenter(const SensorData& data);
  
  /**
   * @brief Avoid obstacle by going right with wall hugging
   * @return Navigation result
   * @note O(n) where n = obstacle size (blocking operation)
   */
  static NavResult avoidObstacleRight();
  
  /**
   * @brief Search for a specific colored line
   * @param targetColor Color to search for
   * @param maxTime Maximum search time in ms
   * @return true if found, false if timeout
   * @note O(n) where n = search time
   */
  static bool searchForLine(Color targetColor, uint16_t maxTime = 2000);
  
  /**
   * @brief Execute a timed turn
   * @param degrees Approximate degrees to turn (positive = right)
   * @param speed Turn speed
   * @note Blocking operation
   */
  static void turn(int16_t degrees, uint8_t speed = SPEED_TURN);
  
  /**
   * @brief Reset navigation state
   */
  static void reset();

private:
  static int8_t searchDirection;   // -1 = left, 1 = right
  static uint8_t searchCount;      // Number of search iterations
  static uint32_t lastStateChange; // Timestamp of last state change
  
  /**
   * @brief Wall-hug along obstacle until cleared
   * @param maxSteps Maximum iterations
   */
  static void wallHugUntilClear(uint8_t maxSteps = 30);
};

#endif // NAVIGATION_H
