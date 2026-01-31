/**
 * @file section2.ino
 * @brief SECTION 2: TARGET SHOOTING
 * @version 2.0 (Optimized)
 * 
 * MISSION:
 * ========
 * 1. Climb ramp to target platform
 * 2. Navigate colored rings: BLUE -> RED -> GREEN -> BLACK (center)
 * 3. Find ball at black center
 * 4. Shoot/launch ball forward
 * 5. Return down ramp
 * 6. Continue to re-upload point
 * 
 * UPLOAD: At first re-upload point (after Section 1)
 * NEXT: Section 3 (Obstacle Course)
 * 
 * TIME COMPLEXITY: O(n) where n = search iterations
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
  STATE_CLIMB_RAMP,
  STATE_ON_TARGET,
  STATE_NAV_BLUE,
  STATE_NAV_RED,
  STATE_NAV_GREEN,
  STATE_REACH_CENTER,
  STATE_FIND_BALL,
  STATE_SHOOT,
  STATE_RETURN_RAMP,
  STATE_TO_REUPLOAD,
  STATE_COMPLETE
};

// ============================================================================
// GLOBALS
// ============================================================================
State currentState = STATE_INIT;
SensorData sensors;
uint32_t stateStartTime = 0;
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
  Serial.println(F("║    SECTION 2: TARGET SHOOTING       ║"));
  Serial.println(F("║  Ramp -> Navigate -> Shoot -> Return║"));
  Serial.println(F("╚═════════════════════════════════════╝"));
  
  currentState = STATE_CLIMB_RAMP;
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
    case STATE_CLIMB_RAMP:   Serial.println(F("CLIMB_RAMP")); break;
    case STATE_ON_TARGET:    Serial.println(F("ON_TARGET")); break;
    case STATE_NAV_BLUE:     Serial.println(F("NAV_BLUE")); break;
    case STATE_NAV_RED:      Serial.println(F("NAV_RED")); break;
    case STATE_NAV_GREEN:    Serial.println(F("NAV_GREEN")); break;
    case STATE_REACH_CENTER: Serial.println(F("REACH_CENTER")); break;
    case STATE_FIND_BALL:    Serial.println(F("FIND_BALL")); break;
    case STATE_SHOOT:        Serial.println(F("SHOOT")); break;
    case STATE_RETURN_RAMP:  Serial.println(F("RETURN_RAMP")); break;
    case STATE_TO_REUPLOAD:  Serial.println(F("TO_REUPLOAD")); break;
    case STATE_COMPLETE:     Serial.println(F("COMPLETE")); break;
    default: break;
  }
}

// ============================================================================
// STATE PROCESSING - O(1) per state
// ============================================================================
void processState() {
  switch (currentState) {
    
    // -------------------------------------------------------------------------
    // STATE: Climb ramp to target platform
    // -------------------------------------------------------------------------
    case STATE_CLIMB_RAMP: {
      Motors::forward(SPEED_FAST);
      
      // Check if we reached target (any colored zone)
      if (sensors.detectedColor != COLOR_NONE && 
          sensors.detectedColor != COLOR_WHITE) {
        Motors::stop();
        transitionTo(STATE_ON_TARGET);
      }
      
      // Timeout: assume we're on target
      if (millis() - stateStartTime > TIMEOUT_RAMP) {
        Motors::stop();
        transitionTo(STATE_ON_TARGET);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Determine position on target
    // -------------------------------------------------------------------------
    case STATE_ON_TARGET: {
      // Route based on detected color
      switch (sensors.detectedColor) {
        case COLOR_BLUE:  transitionTo(STATE_NAV_BLUE); break;
        case COLOR_RED:   transitionTo(STATE_NAV_RED); break;
        case COLOR_GREEN: transitionTo(STATE_NAV_GREEN); break;
        case COLOR_BLACK: transitionTo(STATE_REACH_CENTER); break;
        default:
          // Unknown position - move slowly and re-check
          Motors::forward(SPEED_SLOW);
          delay(200);
          Motors::stop();
          break;
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Navigate from blue zone toward center
    // -------------------------------------------------------------------------
    case STATE_NAV_BLUE: {
      NavResult result = Navigation::navigateToCenter(sensors);
      
      if (result == NAV_TARGET_FOUND) {
        switch (sensors.detectedColor) {
          case COLOR_RED:   transitionTo(STATE_NAV_RED); break;
          case COLOR_GREEN: transitionTo(STATE_NAV_GREEN); break;
          case COLOR_BLACK: transitionTo(STATE_REACH_CENTER); break;
          default: break;
        }
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Navigate from red zone toward center
    // -------------------------------------------------------------------------
    case STATE_NAV_RED: {
      NavResult result = Navigation::navigateToCenter(sensors);
      
      if (result == NAV_TARGET_FOUND) {
        switch (sensors.detectedColor) {
          case COLOR_GREEN: transitionTo(STATE_NAV_GREEN); break;
          case COLOR_BLACK: transitionTo(STATE_REACH_CENTER); break;
          default: break;
        }
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Navigate from green zone toward center
    // -------------------------------------------------------------------------
    case STATE_NAV_GREEN: {
      NavResult result = Navigation::navigateToCenter(sensors);
      
      if (result == NAV_TARGET_FOUND && sensors.detectedColor == COLOR_BLACK) {
        transitionTo(STATE_REACH_CENTER);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Reached black center
    // -------------------------------------------------------------------------
    case STATE_REACH_CENTER: {
      Motors::stop();
      Serial.println(F(">>> CENTER REACHED! <<<"));
      delay(300);
      transitionTo(STATE_FIND_BALL);
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Find ball at center
    // -------------------------------------------------------------------------
    case STATE_FIND_BALL: {
      // Ball should be directly ahead
      if (sensors.distance < DIST_BALL_DETECT && sensors.distance > 0) {
        Serial.println(F(">>> BALL FOUND! <<<"));
        transitionTo(STATE_SHOOT);
      } else {
        // Move slowly to find ball
        Motors::forward(SPEED_SLOW);
        delay(200);
        Motors::stop();
        
        // Timeout
        if (millis() - stateStartTime > TIMEOUT_SEARCH) {
          Serial.println(F(">>> BALL NOT FOUND - SHOOTING ANYWAY <<<"));
          transitionTo(STATE_SHOOT);
        }
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Shoot/launch ball
    // -------------------------------------------------------------------------
    case STATE_SHOOT: {
      Serial.println(F("\n>>> LAUNCHING BALL! <<<"));
      
      // Quick forward push
      Motors::forward(SPEED_FAST);
      delay(400);
      Motors::stop();
      
      // Arm flick for extra power
      Servos::launch();
      
      Serial.println(F(">>> BALL LAUNCHED! <<<"));
      delay(1000);
      
      transitionTo(STATE_RETURN_RAMP);
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Return down ramp
    // -------------------------------------------------------------------------
    case STATE_RETURN_RAMP: {
      // Turn around 180 degrees
      Navigation::turn(180);
      
      // Go down ramp
      Motors::forward(SPEED_NORMAL);
      
      // Timeout: assume we're at bottom
      if (millis() - stateStartTime > 4000) {
        Motors::stop();
        transitionTo(STATE_TO_REUPLOAD);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Go to re-upload point
    // -------------------------------------------------------------------------
    case STATE_TO_REUPLOAD: {
      Motors::stop();
      transitionTo(STATE_COMPLETE);
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Section complete
    // -------------------------------------------------------------------------
    case STATE_COMPLETE: {
      Motors::stop();
      
      Serial.println(F("\n╔═════════════════════════════════════╗"));
      Serial.println(F("║      SECTION 2 COMPLETE!            ║"));
      Serial.println(F("║  Upload section3.ino now            ║"));
      Serial.println(F("╚═════════════════════════════════════╝"));
      
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
  Serial.print(F(" RGB:"));
  Serial.print(sensors.redFreq);
  Serial.print(F(","));
  Serial.print(sensors.greenFreq);
  Serial.print(F(","));
  Serial.print(sensors.blueFreq);
  Serial.print(F(" Dist:"));
  Serial.println(sensors.distance, 1);
}
