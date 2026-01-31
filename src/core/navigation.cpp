/**
 * @file navigation.cpp
 * @brief High-level navigation behaviors implementation
 */

#include "navigation.h"

// Static member initialization
int8_t Navigation::searchDirection = 1;
uint8_t Navigation::searchCount = 0;
uint32_t Navigation::lastStateChange = 0;

// ============================================================================
// BLACK LINE FOLLOWING - O(1)
// ============================================================================
NavResult Navigation::followBlackLine(const SensorData& data) {
  // Check for obstacles first (highest priority)
  if (data.obstacleDetected) {
    Motors::stop();
    return NAV_OBSTACLE;
  }
  
  // Decision tree for line following
  // Both sensors on line -> go straight
  if (data.leftOnLine && data.rightOnLine) {
    Motors::forward(SPEED_NORMAL);
  }
  // Left on, right off -> curve left
  else if (data.leftOnLine && !data.rightOnLine) {
    Motors::curveLeft(SPEED_NORMAL);
  }
  // Right on, left off -> curve right
  else if (!data.leftOnLine && data.rightOnLine) {
    Motors::curveRight(SPEED_NORMAL);
  }
  // Both off -> lost line, move slowly
  else {
    Motors::forward(SPEED_SLOW);
    return NAV_LOST;
  }
  
  return NAV_CONTINUE;
}

// ============================================================================
// COLOR LINE FOLLOWING - O(1)
// ============================================================================
NavResult Navigation::followColorLine(const SensorData& data, Color targetColor) {
  // Check for obstacles
  if (data.obstacleDetected) {
    Motors::stop();
    return NAV_OBSTACLE;
  }
  
  // On target color -> move forward
  if (data.detectedColor == targetColor) {
    Motors::forward(SPEED_NORMAL);
    searchCount = 0;  // Reset search counter
    return NAV_CONTINUE;
  }
  
  // Different color detected -> might be target zone
  if (data.detectedColor != COLOR_NONE && data.detectedColor != targetColor) {
    Motors::stop();
    return NAV_TARGET_FOUND;
  }
  
  // No color / lost line -> search
  Motors::forward(SPEED_SLOW);
  searchCount++;
  
  if (searchCount > 10) {
    return NAV_LOST;
  }
  
  return NAV_CONTINUE;
}

// ============================================================================
// TARGET CENTER NAVIGATION - O(1)
// ============================================================================
NavResult Navigation::navigateToCenter(const SensorData& data) {
  // Move forward slowly while searching
  Motors::forward(SPEED_SLOW);
  delay(100);
  
  // If we found a new (inner) color, success
  if (data.detectedColor != data.previousColor && 
      data.detectedColor != COLOR_NONE) {
    searchCount = 0;
    return NAV_TARGET_FOUND;
  }
  
  // Same color - need to search
  searchCount++;
  
  if (searchCount > 5) {
    // Turn to search for inner ring
    if (searchDirection > 0) {
      Motors::turnRight(SPEED_TURN);
    } else {
      Motors::turnLeft(SPEED_TURN);
    }
    delay(200);
    Motors::stop();
    
    // Alternate direction
    searchDirection *= -1;
    searchCount = 0;
  }
  
  return NAV_CONTINUE;
}

// ============================================================================
// OBSTACLE AVOIDANCE - O(n)
// ============================================================================
NavResult Navigation::avoidObstacleRight() {
  Motors::stop();
  delay(100);
  
  // Step 1: Turn right 90°
  turn(90);
  
  // Step 2: Move forward past obstacle
  Motors::forward(SPEED_NORMAL);
  delay(800);
  
  // Step 3: Turn left 90°
  turn(-90);
  
  // Step 4: Wall-hug along obstacle
  wallHugUntilClear(30);
  
  // Step 5: Turn left to get back on path
  turn(-90);
  
  // Step 6: Clear obstacle
  Motors::forward(SPEED_NORMAL);
  delay(800);
  
  // Step 7: Turn right to resume direction
  turn(90);
  
  Motors::stop();
  return NAV_CONTINUE;
}

void Navigation::wallHugUntilClear(uint8_t maxSteps) {
  for (uint8_t i = 0; i < maxSteps; i++) {
    float dist = Sensors::readDistance();
    
    if (dist < DIST_WALL_HUG - 3) {
      // Too close, veer right
      Motors::curveRight(SPEED_NORMAL);
    }
    else if (dist < DIST_WALL_HUG + 5 && dist > 0) {
      // Good distance, parallel
      Motors::forward(SPEED_NORMAL);
    }
    else if (dist > DIST_WALL_HUG + 10 || dist >= 999) {
      // Lost wall, passed obstacle
      break;
    }
    
    delay(50);
  }
  Motors::stop();
}

// ============================================================================
// LINE SEARCH - O(n)
// ============================================================================
bool Navigation::searchForLine(Color targetColor, uint16_t maxTime) {
  uint32_t startTime = millis();
  SensorData data;
  
  while (millis() - startTime < maxTime) {
    Sensors::readAll(data);
    
    if (data.detectedColor == targetColor) {
      return true;
    }
    
    // Alternate turning left and right
    if (searchDirection > 0) {
      Motors::turnRight(SPEED_TURN);
    } else {
      Motors::turnLeft(SPEED_TURN);
    }
    delay(200);
    Motors::stop();
    
    searchDirection *= -1;
  }
  
  return false;
}

// ============================================================================
// TIMED TURN - O(1)
// ============================================================================
void Navigation::turn(int16_t degrees, uint8_t speed) {
  // Calculate turn time based on degrees
  // TIME_TURN_90 is calibrated for 90 degrees
  uint16_t turnTime = (uint16_t)(abs(degrees) * TIME_TURN_90 / 90);
  
  if (degrees > 0) {
    Motors::turnRight(speed);
  } else {
    Motors::turnLeft(speed);
  }
  
  delay(turnTime);
  Motors::stop();
  delay(50);  // Settling time
}

// ============================================================================
// STATE RESET - O(1)
// ============================================================================
void Navigation::reset() {
  searchDirection = 1;
  searchCount = 0;
  lastStateChange = millis();
}
