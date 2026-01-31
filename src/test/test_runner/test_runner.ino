/**
 * @file test_runner.ino
 * @brief Main test runner for UTRA HACKS robot
 * @details Executes comprehensive test suite for all robot systems
 * 
 * TEST CATEGORIES:
 * ================
 * 1. Sensor Tests
 *    - Ultrasonic (distance measurement)
 *    - Color sensor (TCS3200)
 *    - IR sensors (line detection)
 * 
 * 2. Motor Tests
 *    - Drive motors (forward, backward, turn)
 *    - Servos (clamp, arm)
 * 
 * 3. Navigation Tests
 *    - Line following
 *    - Obstacle avoidance
 *    - Integration tests
 * 
 * USAGE:
 * ======
 * 1. Upload this sketch to your Arduino
 * 2. Open Serial Monitor at 9600 baud
 * 3. Follow on-screen prompts
 * 4. Review test results at the end
 * 
 * CONFIGURATION:
 * ==============
 * Comment out test sections below to skip them
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
#include "../test_framework.h"
#include "../test_framework.cpp"
#include "../test_sensors.h"
#include "../test_sensors.cpp"
#include "../test_motors.h"
#include "../test_motors.cpp"
#include "../test_navigation.h"
#include "../test_navigation.cpp"

// ============================================================================
// TEST CONFIGURATION - Comment out to skip sections
// ============================================================================
#define RUN_SENSOR_TESTS
#define RUN_MOTOR_TESTS
#define RUN_NAVIGATION_TESTS

// ============================================================================
// SETUP
// ============================================================================
void setup() {
  // Initialize test framework (starts Serial)
  TestFramework::init(9600);
  
  // Initialize all hardware
  Serial.println(F("Initializing hardware..."));
  Sensors::init();
  Motors::init();
  Servos::init();
  
  Serial.println(F("Hardware initialized successfully!\n"));
  
  // Display test menu
  printTestMenu();
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
  // Wait for user to select test option
  if (Serial.available()) {
    char choice = Serial.read();
    
    // Clear any remaining input
    while (Serial.available()) Serial.read();
    
    switch (choice) {
      case '1':
        runSensorTests();
        break;
      case '2':
        runMotorTests();
        break;
      case '3':
        runNavigationTests();
        break;
      case '4':
        runAllTests();
        break;
      case '5':
        runQuickDiagnostics();
        break;
      case '6':
        runCalibration();
        break;
      case 'r':
      case 'R':
        TestFramework::reset();
        printTestMenu();
        break;
      default:
        Serial.println(F("Invalid option. Press 1-6 or R."));
        break;
    }
  }
  
  delay(100);
}

// ============================================================================
// TEST MENU
// ============================================================================
void printTestMenu() {
  Serial.println(F("\n╔═══════════════════════════════════════════════════════════╗"));
  Serial.println(F("║                    TEST MENU                              ║"));
  Serial.println(F("╠═══════════════════════════════════════════════════════════╣"));
  Serial.println(F("║  1. Sensor Tests (Ultrasonic, Color, IR)                  ║"));
  Serial.println(F("║  2. Motor Tests (Drive motors, Servos)                    ║"));
  Serial.println(F("║  3. Navigation Tests (Line following, Obstacle avoid)    ║"));
  Serial.println(F("║  4. Run ALL Tests                                         ║"));
  Serial.println(F("║  5. Quick Diagnostics (fast hardware check)               ║"));
  Serial.println(F("║  6. Calibration Mode                                      ║"));
  Serial.println(F("║                                                           ║"));
  Serial.println(F("║  R. Reset counters                                        ║"));
  Serial.println(F("╚═══════════════════════════════════════════════════════════╝"));
  Serial.println(F("\nEnter your choice (1-6):"));
}

// ============================================================================
// TEST RUNNERS
// ============================================================================
void runSensorTests() {
  #ifdef RUN_SENSOR_TESTS
  Serial.println(F("\n>>> RUNNING SENSOR TESTS <<<\n"));
  TestSensors::runAll();
  TestFramework::printSummary();
  printTestMenu();
  #else
  Serial.println(F("Sensor tests disabled in configuration"));
  #endif
}

void runMotorTests() {
  #ifdef RUN_MOTOR_TESTS
  Serial.println(F("\n>>> RUNNING MOTOR TESTS <<<\n"));
  TestMotors::runAll();
  TestFramework::printSummary();
  printTestMenu();
  #else
  Serial.println(F("Motor tests disabled in configuration"));
  #endif
}

void runNavigationTests() {
  #ifdef RUN_NAVIGATION_TESTS
  Serial.println(F("\n>>> RUNNING NAVIGATION TESTS <<<\n"));
  TestNavigation::runAll();
  TestFramework::printSummary();
  printTestMenu();
  #else
  Serial.println(F("Navigation tests disabled in configuration"));
  #endif
}

void runAllTests() {
  Serial.println(F("\n>>> RUNNING COMPLETE TEST SUITE <<<\n"));
  TestFramework::reset();
  
  #ifdef RUN_SENSOR_TESTS
  TestSensors::runAll();
  #endif
  
  #ifdef RUN_MOTOR_TESTS
  TestMotors::runAll();
  #endif
  
  #ifdef RUN_NAVIGATION_TESTS
  TestNavigation::runAll();
  #endif
  
  TestFramework::printSummary();
  printTestMenu();
}

// ============================================================================
// QUICK DIAGNOSTICS
// ============================================================================
void runQuickDiagnostics() {
  Serial.println(F("\n>>> QUICK DIAGNOSTICS <<<\n"));
  Serial.println(F("Testing all hardware without user interaction...\n"));
  
  // Ultrasonic
  Serial.print(F("Ultrasonic: "));
  float dist = Sensors::readDistance();
  if (dist > 0 && dist < 999) {
    Serial.print(dist); Serial.println(F(" cm - OK"));
  } else {
    Serial.println(F("TIMEOUT or ERROR"));
  }
  
  // Color sensor
  Serial.print(F("Color sensor: "));
  uint16_t r, g, b;
  Sensors::readColorRaw(r, g, b);
  if (r < 999 && g < 999 && b < 999) {
    Serial.print(F("R=")); Serial.print(r);
    Serial.print(F(" G=")); Serial.print(g);
    Serial.print(F(" B=")); Serial.print(b);
    Serial.print(F(" -> ")); Serial.println(Sensors::colorName(Sensors::detectColor(r, g, b)));
  } else {
    Serial.println(F("ERROR"));
  }
  
  // IR sensors
  Serial.print(F("IR sensors: "));
  bool left, right;
  Sensors::readIR(left, right);
  Serial.print(F("L=")); Serial.print(left ? "ON" : "OFF");
  Serial.print(F(" R=")); Serial.println(right ? "ON" : "OFF");
  
  // Motors (brief test)
  Serial.print(F("Motors: "));
  Motors::forward(100);
  delay(200);
  Motors::stop();
  Serial.println(F("Pulsed - check visually"));
  
  // Servos
  Serial.print(F("Base servo: "));
  Servos::armCarry();
  Serial.println(F("Moved to carry"));
  
  Serial.print(F("Clamp servo: "));
  Servos::openClamp();
  Serial.println(F("Opened"));
  
  Serial.println(F("\n>>> DIAGNOSTICS COMPLETE <<<"));
  printTestMenu();
}

// ============================================================================
// CALIBRATION MODE
// ============================================================================
void runCalibration() {
  Serial.println(F("\n>>> CALIBRATION MODE <<<\n"));
  Serial.println(F("This mode displays real-time sensor values."));
  Serial.println(F("Use to calibrate thresholds in constants.h"));
  Serial.println(F("Press any key to exit.\n"));
  
  delay(1000);
  
  while (!Serial.available()) {
    SensorData data;
    Sensors::readAll(data);
    
    // Clear line and print
    Serial.print(F("\r"));
    Serial.print(F("Dist: ")); Serial.print(data.distance, 1); Serial.print(F("cm | "));
    Serial.print(F("RGB: ")); Serial.print(data.redFreq); Serial.print(F(","));
    Serial.print(data.greenFreq); Serial.print(F(",")); Serial.print(data.blueFreq);
    Serial.print(F(" [")); Serial.print(Sensors::colorName(data.detectedColor)); Serial.print(F("] | "));
    Serial.print(F("IR: ")); Serial.print(data.leftOnLine ? "L" : "-");
    Serial.print(data.rightOnLine ? "R" : "-");
    Serial.print(F("    "));  // Clear any leftover characters
    
    delay(200);
  }
  
  // Clear input
  while (Serial.available()) Serial.read();
  
  Serial.println(F("\n\n>>> CALIBRATION MODE ENDED <<<"));
  printTestMenu();
}
