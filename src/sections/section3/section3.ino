/**
 * @file section3.ino
 * @brief SECTION 3: OBSTACLE COURSE
 * @version 2.0 (Optimized)
 * 
 * MISSION:
 * ========
 * 1. At intersection, take LEFT-most red line
 * 2. Pick up box along path
 * 3. Navigate through two obstructions using wall-hugging
 * 4. Pass re-upload point (continue holding box)
 * 5. Drop box at blue zone
 * 6. Return to start via black line
 * 
 * UPLOAD: At second re-upload point (after Section 2)
 * NEXT: Competition complete!
 * 
 * TIME COMPLEXITY: O(n) where n = path length + obstacles
 * SPACE COMPLEXITY: O(1) - constant memory usage
 */

#include "../../config/pins.h"
#include "../../config/constants.h"
#include "../../core/sensors.h"
#include "../../core/sensors.cpp"
#include "../../core/motors.h"
#include "../../core/motors.cpp"
#include "../../core/servos.h"
#include "../../core/servos.cpp"
#include "../../core/navigation.h"
#include "../../core/navigation.cpp"

// ============================================================================
// STATE MACHINE
// ============================================================================
enum State : uint8_t {
  STATE_INIT,
  STATE_FIND_RED_LINE,
  STATE_FOLLOW_RED,
  STATE_APPROACH_BOX,
  STATE_PICKUP,
  STATE_TO_OBSTACLES,
  STATE_AVOID_OBS_1,
  STATE_CLEAR_OBS_1,
  STATE_AVOID_OBS_2,
  STATE_CLEAR_OBS_2,
  STATE_PASS_REUPLOAD,
  STATE_FIND_BLUE,
  STATE_DROP,
  STATE_RETURN_INTERSECTION,
  STATE_FIND_BLACK,
  STATE_FOLLOW_BLACK_HOME,
  STATE_COMPLETE
};

// ============================================================================
// GLOBALS
// ============================================================================
State currentState = STATE_INIT;
SensorData sensors;
uint32_t stateStartTime = 0;
uint8_t obstacleCount = 0;
bool debugEnabled = true;

// ============================================================================
// SETUP - O(1)
// ============================================================================
void setup() {
  Serial.begin(9600);
  
  Sensors::init();
  Motors::init();
  Servos::init();
  Navigation::reset();
  
  delay(500);
  
  Serial.println(F("╔═════════════════════════════════════╗"));
  Serial.println(F("║    SECTION 3: OBSTACLE COURSE       ║"));
  Serial.println(F("║  Red line -> Obstacles -> Return    ║"));
  Serial.println(F("╚═════════════════════════════════════╝"));
  
  currentState = STATE_FIND_RED_LINE;
  stateStartTime = millis();
}

// ============================================================================
// MAIN LOOP - O(1) per iteration
// ============================================================================
void loop() {
  Sensors::readAll(sensors);
  processState();
  
  if (debugEnabled) {
    debugPrint();
  }
  
  delay(TIME_LOOP_DELAY);
}

// ============================================================================
// STATE TRANSITION - O(1)
// ============================================================================
void transitionTo(State newState) {
  currentState = newState;
  stateStartTime = millis();
  Navigation::reset();
  
  Serial.print(F("\n>> STATE: "));
  switch (newState) {
    case STATE_FIND_RED_LINE:      Serial.println(F("FIND_RED_LINE")); break;
    case STATE_FOLLOW_RED:         Serial.println(F("FOLLOW_RED")); break;
    case STATE_APPROACH_BOX:       Serial.println(F("APPROACH_BOX")); break;
    case STATE_PICKUP:             Serial.println(F("PICKUP")); break;
    case STATE_TO_OBSTACLES:       Serial.println(F("TO_OBSTACLES")); break;
    case STATE_AVOID_OBS_1:        Serial.println(F("AVOID_OBS_1")); break;
    case STATE_CLEAR_OBS_1:        Serial.println(F("CLEAR_OBS_1")); break;
    case STATE_AVOID_OBS_2:        Serial.println(F("AVOID_OBS_2")); break;
    case STATE_CLEAR_OBS_2:        Serial.println(F("CLEAR_OBS_2")); break;
    case STATE_PASS_REUPLOAD:      Serial.println(F("PASS_REUPLOAD")); break;
    case STATE_FIND_BLUE:          Serial.println(F("FIND_BLUE")); break;
    case STATE_DROP:               Serial.println(F("DROP")); break;
    case STATE_RETURN_INTERSECTION:Serial.println(F("RETURN_INTERSECTION")); break;
    case STATE_FIND_BLACK:         Serial.println(F("FIND_BLACK")); break;
    case STATE_FOLLOW_BLACK_HOME:  Serial.println(F("FOLLOW_BLACK_HOME")); break;
    case STATE_COMPLETE:           Serial.println(F("COMPLETE")); break;
    default: break;
  }
}

// ============================================================================
// STATE PROCESSING - O(1) per state
// ============================================================================
void processState() {
  switch (currentState) {
    
    // -------------------------------------------------------------------------
    // STATE: Find left red line at intersection
    // -------------------------------------------------------------------------
    case STATE_FIND_RED_LINE: {
      // Turn left to find red line (left-most path)
      Navigation::turn(-90);  // Turn left
      
      Sensors::readAll(sensors);
      
      if (sensors.detectedColor == COLOR_RED) {
        transitionTo(STATE_FOLLOW_RED);
      } else {
        // Search forward
        Motors::forward(SPEED_SLOW);
        delay(300);
        Motors::stop();
        
        if (millis() - stateStartTime > 3000) {
          // Force transition
          transitionTo(STATE_FOLLOW_RED);
        }
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Follow red line
    // -------------------------------------------------------------------------
    case STATE_FOLLOW_RED: {
      NavResult result = Navigation::followColorLine(sensors, COLOR_RED);
      
      // Check for box (if not already holding)
      if (!Servos::isHolding() && 
          sensors.obstacleDetected && 
          sensors.distance < DIST_BOX_PICKUP + 10) {
        transitionTo(STATE_APPROACH_BOX);
      }
      
      // Check for obstacle (if holding box)
      if (Servos::isHolding() && 
          sensors.obstacleDetected && 
          sensors.distance < DIST_OBSTACLE) {
        transitionTo(STATE_TO_OBSTACLES);
      }
      
      // Check for blue zone (near end)
      if (Servos::isHolding() && 
          obstacleCount >= 2 && 
          sensors.detectedColor == COLOR_BLUE) {
        transitionTo(STATE_FIND_BLUE);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Approach box for pickup
    // -------------------------------------------------------------------------
    case STATE_APPROACH_BOX: {
      if (sensors.distance <= DIST_BOX_PICKUP) {
        Motors::stop();
        transitionTo(STATE_PICKUP);
      } else {
        Motors::forward(SPEED_SLOW);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Pickup box
    // -------------------------------------------------------------------------
    case STATE_PICKUP: {
      Servos::pickup();
      
      if (Servos::isHolding()) {
        transitionTo(STATE_TO_OBSTACLES);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Navigate toward obstacles
    // -------------------------------------------------------------------------
    case STATE_TO_OBSTACLES: {
      Navigation::followColorLine(sensors, COLOR_RED);
      
      if (sensors.obstacleDetected && sensors.distance < DIST_OBSTACLE) {
        obstacleCount++;
        
        if (obstacleCount == 1) {
          transitionTo(STATE_AVOID_OBS_1);
        } else {
          transitionTo(STATE_AVOID_OBS_2);
        }
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Avoid first obstacle
    // -------------------------------------------------------------------------
    case STATE_AVOID_OBS_1: {
      Serial.println(F(">>> AVOIDING OBSTACLE 1 <<<"));
      Navigation::avoidObstacleRight();
      transitionTo(STATE_CLEAR_OBS_1);
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Clear first obstacle and continue
    // -------------------------------------------------------------------------
    case STATE_CLEAR_OBS_1: {
      Motors::forward(SPEED_NORMAL);
      delay(500);
      
      // Search for red line
      if (Navigation::searchForLine(COLOR_RED, 1000)) {
        transitionTo(STATE_TO_OBSTACLES);
      } else {
        Motors::forward(SPEED_SLOW);
        delay(500);
        transitionTo(STATE_TO_OBSTACLES);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Avoid second obstacle
    // -------------------------------------------------------------------------
    case STATE_AVOID_OBS_2: {
      Serial.println(F(">>> AVOIDING OBSTACLE 2 <<<"));
      Navigation::avoidObstacleRight();
      transitionTo(STATE_CLEAR_OBS_2);
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Clear second obstacle and continue
    // -------------------------------------------------------------------------
    case STATE_CLEAR_OBS_2: {
      Motors::forward(SPEED_NORMAL);
      delay(500);
      transitionTo(STATE_PASS_REUPLOAD);
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Pass re-upload point (continue with box)
    // -------------------------------------------------------------------------
    case STATE_PASS_REUPLOAD: {
      Navigation::followColorLine(sensors, COLOR_RED);
      
      // Look for blue zone
      if (sensors.detectedColor == COLOR_BLUE) {
        transitionTo(STATE_FIND_BLUE);
      }
      
      // Timeout - start searching
      if (millis() - stateStartTime > 5000) {
        if (!Navigation::searchForLine(COLOR_BLUE, 2000)) {
          Navigation::searchForLine(COLOR_RED, 1000);
        }
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Found blue zone
    // -------------------------------------------------------------------------
    case STATE_FIND_BLUE: {
      Motors::forward(SPEED_SLOW);
      delay(500);
      Motors::stop();
      transitionTo(STATE_DROP);
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Drop box at blue zone
    // -------------------------------------------------------------------------
    case STATE_DROP: {
      Servos::drop();
      
      if (!Servos::isHolding()) {
        transitionTo(STATE_RETURN_INTERSECTION);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Return toward intersection
    // -------------------------------------------------------------------------
    case STATE_RETURN_INTERSECTION: {
      // Follow available path back
      if (sensors.detectedColor == COLOR_RED) {
        Navigation::followColorLine(sensors, COLOR_RED);
      } else {
        Motors::forward(SPEED_SLOW);
      }
      
      // Look for black line
      if (sensors.detectedColor == COLOR_BLACK) {
        transitionTo(STATE_FIND_BLACK);
      }
      
      // Timeout
      if (millis() - stateStartTime > 5000) {
        transitionTo(STATE_FIND_BLACK);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Find black line for return
    // -------------------------------------------------------------------------
    case STATE_FIND_BLACK: {
      if (sensors.detectedColor == COLOR_BLACK) {
        transitionTo(STATE_FOLLOW_BLACK_HOME);
      } else {
        // Search for black line
        Navigation::turn(45);
        Sensors::readAll(sensors);
        
        if (sensors.detectedColor != COLOR_BLACK) {
          Navigation::turn(-90);
        }
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Follow black line back to start
    // -------------------------------------------------------------------------
    case STATE_FOLLOW_BLACK_HOME: {
      Navigation::followBlackLine(sensors);
      
      // Timeout - assume we're at start
      if (millis() - stateStartTime > TIMEOUT_RETURN) {
        Motors::stop();
        transitionTo(STATE_COMPLETE);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Competition complete!
    // -------------------------------------------------------------------------
    case STATE_COMPLETE: {
      Motors::stop();
      
      Serial.println(F("\n╔═══════════════════════════════════════════════╗"));
      Serial.println(F("║         COMPETITION COMPLETE!                 ║"));
      Serial.println(F("║                                               ║"));
      Serial.println(F("║  ★★★ ALL SECTIONS FINISHED ★★★              ║"));
      Serial.println(F("║                                               ║"));
      Serial.println(F("║  Great job team!                              ║"));
      Serial.println(F("╚═══════════════════════════════════════════════╝"));
      
      // Victory dance
      for (int i = 0; i < 3; i++) {
        Servos::openClamp();
        delay(200);
        Servos::closeClamp();
        delay(200);
      }
      Servos::openClamp();
      
      while (true) {
        delay(1000);
      }
      break;
    }
  }
}

// ============================================================================
// DEBUG OUTPUT - O(1)
// ============================================================================
void debugPrint() {
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint < 500) return;
  lastPrint = millis();
  
  Serial.print(F("Color:"));
  Serial.print(Sensors::colorName(sensors.detectedColor));
  Serial.print(F(" Dist:"));
  Serial.print(sensors.distance, 1);
  Serial.print(F(" Hold:"));
  Serial.print(Servos::isHolding() ? "Y" : "N");
  Serial.print(F(" Obs:"));
  Serial.println(obstacleCount);
}
