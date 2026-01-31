/**
 * @file section1.ino
 * @brief SECTION 1: START SECTION
 * @version 2.0 (Optimized)
 * 
 * MISSION:
 * ========
 * 1. Follow black line forward
 * 2. Detect and pick up box
 * 3. At intersection, choose GREEN path (not red)
 * 4. Follow green line
 * 5. Detect blue zone and drop box
 * 6. Continue to re-upload point
 * 
 * UPLOAD: At competition START
 * NEXT: Section 2 (Target Shooting)
 * 
 * TIME COMPLEXITY: O(n) where n = path length
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
  STATE_FOLLOW_BLACK,
  STATE_APPROACH_BOX,
  STATE_PICKUP,
  STATE_FIND_INTERSECTION,
  STATE_SELECT_GREEN,
  STATE_FOLLOW_GREEN,
  STATE_APPROACH_BLUE,
  STATE_DROP,
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
// FUNCTION PROTOTYPES
// ============================================================================
void transitionTo(State newState);
void processState();
void debugPrint();

// ============================================================================
// SETUP - O(1)
// ============================================================================
void setup() {
  Serial.begin(9600);
  
  // Initialize all subsystems
  Sensors::init();
  Motors::init();
  Servos::init();
  Navigation::reset();
  
  delay(500);
  
  Serial.println(F("╔═════════════════════════════════════╗"));
  Serial.println(F("║     SECTION 1: START SECTION        ║"));
  Serial.println(F("║  Black line -> Box -> Green -> Blue ║"));
  Serial.println(F("╚═════════════════════════════════════╝"));
  
  transitionTo(STATE_FOLLOW_BLACK);
}

// ============================================================================
// MAIN LOOP - O(1) per iteration
// ============================================================================
void loop() {
  // Read all sensors once per loop - O(1)
  Sensors::readAll(sensors);
  
  // Process current state - O(1)
  processState();
  
  // Debug output (optional)
  if (debugEnabled) {
    debugPrint();
  }
  
  // Fixed loop delay
  delay(TIME_LOOP_DELAY);
}

// ============================================================================
// STATE TRANSITION - O(1)
// ============================================================================
void transitionTo(State newState) {
  currentState = newState;
  stateStartTime = millis();
  Navigation::reset();
  
  // Log transition
  Serial.print(F("\n>> STATE: "));
  switch (newState) {
    case STATE_INIT:           Serial.println(F("INIT")); break;
    case STATE_FOLLOW_BLACK:   Serial.println(F("FOLLOW_BLACK")); break;
    case STATE_APPROACH_BOX:   Serial.println(F("APPROACH_BOX")); break;
    case STATE_PICKUP:         Serial.println(F("PICKUP")); break;
    case STATE_FIND_INTERSECTION: Serial.println(F("FIND_INTERSECTION")); break;
    case STATE_SELECT_GREEN:   Serial.println(F("SELECT_GREEN")); break;
    case STATE_FOLLOW_GREEN:   Serial.println(F("FOLLOW_GREEN")); break;
    case STATE_APPROACH_BLUE:  Serial.println(F("APPROACH_BLUE")); break;
    case STATE_DROP:           Serial.println(F("DROP")); break;
    case STATE_TO_REUPLOAD:    Serial.println(F("TO_REUPLOAD")); break;
    case STATE_COMPLETE:       Serial.println(F("COMPLETE")); break;
  }
}

// ============================================================================
// STATE PROCESSING - O(1) per state
// ============================================================================
void processState() {
  switch (currentState) {
    
    // -------------------------------------------------------------------------
    // STATE: Follow black line looking for box
    // -------------------------------------------------------------------------
    case STATE_FOLLOW_BLACK: {
      NavResult result = Navigation::followBlackLine(sensors);
      
      // Check for obstacle (box)
      if (result == NAV_OBSTACLE && sensors.distance < DIST_BOX_PICKUP + 10) {
        transitionTo(STATE_APPROACH_BOX);
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
    // STATE: Execute pickup sequence
    // -------------------------------------------------------------------------
    case STATE_PICKUP: {
      Servos::pickup();
      
      if (Servos::isHolding()) {
        transitionTo(STATE_FIND_INTERSECTION);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Continue following black line to intersection
    // -------------------------------------------------------------------------
    case STATE_FIND_INTERSECTION: {
      Navigation::followBlackLine(sensors);
      
      // Check for colored line (intersection)
      if (sensors.detectedColor == COLOR_GREEN || 
          sensors.detectedColor == COLOR_RED) {
        Motors::stop();
        transitionTo(STATE_SELECT_GREEN);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Select green path at intersection
    // -------------------------------------------------------------------------
    case STATE_SELECT_GREEN: {
      // If already on green, proceed
      if (sensors.detectedColor == COLOR_GREEN) {
        transitionTo(STATE_FOLLOW_GREEN);
        break;
      }
      
      // Search for green - turn left first (green is typically left)
      static int8_t searchDir = -1;  // -1 = left first
      static uint8_t searchAttempts = 0;
      
      if (searchAttempts < 4) {
        Navigation::turn(45 * searchDir);
        Sensors::readAll(sensors);
        
        if (sensors.detectedColor == COLOR_GREEN) {
          transitionTo(STATE_FOLLOW_GREEN);
          searchAttempts = 0;
          searchDir = -1;
        } else {
          searchDir *= -1;  // Alternate direction
          searchAttempts++;
        }
      } else {
        // Fallback: just go forward
        transitionTo(STATE_FOLLOW_GREEN);
        searchAttempts = 0;
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Follow green line to blue zone
    // -------------------------------------------------------------------------
    case STATE_FOLLOW_GREEN: {
      NavResult result = Navigation::followColorLine(sensors, COLOR_GREEN);
      
      // Check for blue zone
      if (sensors.detectedColor == COLOR_BLUE) {
        transitionTo(STATE_APPROACH_BLUE);
      }
      
      // Handle lost line
      if (result == NAV_LOST) {
        Navigation::searchForLine(COLOR_GREEN, 1000);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Move into blue zone
    // -------------------------------------------------------------------------
    case STATE_APPROACH_BLUE: {
      // Move fully into blue zone
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
        transitionTo(STATE_TO_REUPLOAD);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Continue to re-upload point
    // -------------------------------------------------------------------------
    case STATE_TO_REUPLOAD: {
      // Continue on green line toward re-upload point
      Navigation::followColorLine(sensors, COLOR_GREEN);
      
      // Timeout after 3 seconds - assume we've reached re-upload
      if (millis() - stateStartTime > 3000) {
        Motors::stop();
        transitionTo(STATE_COMPLETE);
      }
      break;
    }
    
    // -------------------------------------------------------------------------
    // STATE: Section complete
    // -------------------------------------------------------------------------
    case STATE_COMPLETE: {
      Motors::stop();
      
      Serial.println(F("\n╔═════════════════════════════════════╗"));
      Serial.println(F("║      SECTION 1 COMPLETE!            ║"));
      Serial.println(F("║  Upload section2.ino now            ║"));
      Serial.println(F("╚═════════════════════════════════════╝"));
      
      // Halt
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
  
  // Print every 500ms
  if (millis() - lastPrint < 500) return;
  lastPrint = millis();
  
  Serial.print(F("Color:"));
  Serial.print(Sensors::colorName(sensors.detectedColor));
  Serial.print(F(" Dist:"));
  Serial.print(sensors.distance, 1);
  Serial.print(F(" IR:"));
  Serial.print(sensors.leftOnLine ? "L" : "-");
  Serial.print(sensors.rightOnLine ? "R" : "-");
  Serial.print(F(" Hold:"));
  Serial.println(Servos::isHolding() ? "Y" : "N");
}
