/**
 * @file test_navigation.cpp
 * @brief Navigation behavior tests implementation
 */

#include "test_navigation.h"

namespace TestNavigation {

void runAll() {
  LineFollowing::runAll();
  ObstacleAvoidance::runAll();
  Integration::runAll();
}

// ============================================================================
// LINE FOLLOWING TESTS
// ============================================================================
namespace LineFollowing {

void runAll() {
  TEST_SECTION("Line Following");
  
  Serial.println(F("\n  ⚠️  NOTE: Place robot on test track for these tests"));
  TestFramework::waitForInput(F("Ready to test line following?"));
  
  testStraightLine();
  testGentleCurve();
  testSharpTurn();
  testLineRecovery();
  testColorLine();
}

void testStraightLine() {
  TEST_CASE("Straight black line following");
  TestFramework::waitForInput(F("Place robot at start of STRAIGHT black line"));
  TestFramework::countdown(3, F("Starting"));
  
  Navigation::reset();
  SensorData data;
  
  unsigned long startTime = millis();
  int successCount = 0;
  int totalSteps = 0;
  
  while (millis() - startTime < 5000) {
    Sensors::readAll(data);
    NavResult result = Navigation::followBlackLine(data);
    
    if (result == NAV_CONTINUE) {
      successCount++;
    }
    totalSteps++;
    
    delay(TIME_LOOP_DELAY);
  }
  
  Motors::stop();
  
  float successRate = (float)successCount / totalSteps * 100;
  Serial.print(F("    Success rate: ")); Serial.print(successRate); Serial.println(F("%"));
  
  TEST_ASSERT(successRate > 70, "Should follow line >70% of time");
}

void testGentleCurve() {
  TEST_CASE("Gentle curve following");
  TestFramework::waitForInput(F("Place robot at start of CURVED section"));
  TestFramework::countdown(3, F("Starting"));
  
  Navigation::reset();
  SensorData data;
  
  unsigned long startTime = millis();
  int lostCount = 0;
  
  while (millis() - startTime < 5000) {
    Sensors::readAll(data);
    NavResult result = Navigation::followBlackLine(data);
    
    if (result == NAV_LOST) {
      lostCount++;
    }
    
    delay(TIME_LOOP_DELAY);
  }
  
  Motors::stop();
  
  Serial.print(F("    Times lost: ")); Serial.println(lostCount);
  TEST_ASSERT(lostCount < 10, "Should not lose line frequently on curves");
}

void testSharpTurn() {
  TEST_CASE("Sharp turn handling");
  TestFramework::waitForInput(F("Place robot before a 90-degree turn"));
  TestFramework::countdown(3, F("Starting"));
  
  Navigation::reset();
  SensorData data;
  
  bool completedTurn = false;
  unsigned long startTime = millis();
  
  while (millis() - startTime < 8000) {
    Sensors::readAll(data);
    Navigation::followBlackLine(data);
    
    delay(TIME_LOOP_DELAY);
  }
  
  Motors::stop();
  
  Serial.println(F("    Did robot navigate the sharp turn?"));
  TEST_ASSERT(true, "Sharp turn test executed");
}

void testLineRecovery() {
  TEST_CASE("Line recovery when lost");
  TestFramework::waitForInput(F("Place robot NEXT TO (not on) black line"));
  TestFramework::countdown(3, F("Starting search"));
  
  Navigation::reset();
  SensorData data;
  
  bool foundLine = false;
  unsigned long startTime = millis();
  
  while (millis() - startTime < 5000) {
    Sensors::readAll(data);
    
    if (data.leftOnLine || data.rightOnLine) {
      foundLine = true;
      Serial.println(F("    Line found!"));
      break;
    }
    
    // Simple search: move forward slowly and turn
    Motors::forward(SPEED_SLOW);
    delay(200);
    Motors::turnLeft(SPEED_TURN);
    delay(100);
  }
  
  Motors::stop();
  
  TEST_ASSERT(foundLine, "Should recover and find the line");
}

void testColorLine() {
  TEST_CASE("Colored line following (GREEN)");
  TestFramework::waitForInput(F("Place robot at start of GREEN line"));
  TestFramework::countdown(3, F("Starting"));
  
  Navigation::reset();
  SensorData data;
  
  unsigned long startTime = millis();
  bool sawGreen = false;
  
  while (millis() - startTime < 5000) {
    Sensors::readAll(data);
    NavResult result = Navigation::followColorLine(data, COLOR_GREEN);
    
    if (data.detectedColor == COLOR_GREEN) {
      sawGreen = true;
    }
    
    if (result == NAV_TARGET_FOUND) {
      Serial.println(F("    Different color detected!"));
      break;
    }
    
    delay(TIME_LOOP_DELAY);
  }
  
  Motors::stop();
  
  TEST_ASSERT(sawGreen, "Should detect green while following");
}

} // namespace LineFollowing

// ============================================================================
// OBSTACLE AVOIDANCE TESTS
// ============================================================================
namespace ObstacleAvoidance {

void runAll() {
  TEST_SECTION("Obstacle Avoidance");
  
  Serial.println(F("\n  ⚠️  NOTE: You will need obstacles for these tests"));
  TestFramework::waitForInput(F("Ready to test obstacle avoidance?"));
  
  testObstacleDetection();
  testAvoidanceManeuver();
  testWallHugging();
}

void testObstacleDetection() {
  TEST_CASE("Obstacle detection");
  TestFramework::waitForInput(F("Place obstacle ~15cm in front of robot"));
  
  SensorData data;
  Sensors::readAll(data);
  
  Serial.print(F("    Distance: ")); Serial.print(data.distance); Serial.println(F(" cm"));
  Serial.print(F("    Obstacle detected: ")); Serial.println(data.obstacleDetected ? "YES" : "NO");
  
  TEST_ASSERT(data.obstacleDetected, "Should detect obstacle");
  TEST_ASSERT_RANGE(data.distance, 5.0f, 25.0f, "Distance in expected range");
}

void testAvoidanceManeuver() {
  TEST_CASE("Obstacle avoidance maneuver");
  
  Serial.println(F("    Place box obstacle on test track"));
  TestFramework::waitForInput(F("Place robot facing obstacle at ~20cm"));
  TestFramework::countdown(3, F("Executing avoidance"));
  
  // Move forward until obstacle detected
  SensorData data;
  while (true) {
    Sensors::readAll(data);
    if (data.obstacleDetected) {
      Motors::stop();
      break;
    }
    Motors::forward(SPEED_NORMAL);
    delay(50);
  }
  
  Serial.println(F("    Obstacle detected - executing avoidance"));
  
  Navigation::avoidObstacleRight();
  
  Serial.println(F("    Did robot successfully navigate around obstacle?"));
  TEST_ASSERT(true, "Avoidance maneuver executed");
}

void testWallHugging() {
  TEST_CASE("Wall hugging behavior");
  
  Serial.println(F("    Test the robot's ability to follow a wall"));
  TestFramework::waitForInput(F("Place robot parallel to wall, ~10cm away"));
  TestFramework::countdown(3, F("Starting wall following"));
  
  unsigned long startTime = millis();
  float minDist = 999, maxDist = 0;
  
  while (millis() - startTime < 5000) {
    float dist = Sensors::readDistance();
    
    if (dist < 999) {
      if (dist < minDist) minDist = dist;
      if (dist > maxDist) maxDist = dist;
    }
    
    // Wall hug logic
    if (dist < DIST_WALL_HUG - 2) {
      Motors::curveRight(SPEED_NORMAL);
    } else if (dist < DIST_WALL_HUG + 2) {
      Motors::forward(SPEED_NORMAL);
    } else {
      Motors::curveLeft(SPEED_NORMAL);
    }
    
    delay(50);
  }
  
  Motors::stop();
  
  Serial.print(F("    Distance range: ")); Serial.print(minDist);
  Serial.print(F(" - ")); Serial.print(maxDist); Serial.println(F(" cm"));
  
  float variation = maxDist - minDist;
  TEST_ASSERT(variation < 15, "Wall distance should stay relatively constant");
}

} // namespace ObstacleAvoidance

// ============================================================================
// INTEGRATION TESTS
// ============================================================================
namespace Integration {

void runAll() {
  TEST_SECTION("Integration Tests");
  
  Serial.println(F("\n  These tests combine multiple behaviors"));
  TestFramework::waitForInput(F("Ready for integration tests?"));
  
  testPickupOnLine();
  testPathSelection();
  testSection1Sequence();
}

void testPickupOnLine() {
  TEST_CASE("Pickup box while line following");
  
  Serial.println(F("    Place box on black line path"));
  TestFramework::waitForInput(F("Place robot at start, box ~30cm ahead"));
  TestFramework::countdown(3, F("Starting"));
  
  Navigation::reset();
  SensorData data;
  bool pickedUp = false;
  
  while (!pickedUp) {
    Sensors::readAll(data);
    
    // Follow line
    Navigation::followBlackLine(data);
    
    // Check for box
    if (data.obstacleDetected && data.distance < DIST_BOX_PICKUP + 3) {
      Motors::stop();
      delay(200);
      
      // Move closer
      while (data.distance > DIST_BOX_PICKUP) {
        Motors::forward(SPEED_SLOW);
        delay(100);
        Sensors::readAll(data);
      }
      Motors::stop();
      
      // Pickup
      Servos::pickup();
      pickedUp = true;
    }
    
    delay(TIME_LOOP_DELAY);
  }
  
  TEST_ASSERT(Servos::isHolding(), "Should be holding box");
  
  // Drop it for next test
  Servos::drop();
}

void testPathSelection() {
  TEST_CASE("Path selection at intersection");
  
  Serial.println(F("    Test choosing green vs red path"));
  TestFramework::waitForInput(F("Place robot at GREEN/RED intersection"));
  TestFramework::countdown(3, F("Starting"));
  
  SensorData data;
  Sensors::readAll(data);
  
  // Search for green
  bool foundGreen = false;
  
  // Turn left first (looking for green)
  Motors::turnLeft(SPEED_TURN);
  for (int i = 0; i < 10; i++) {
    Sensors::readAll(data);
    if (data.detectedColor == COLOR_GREEN) {
      foundGreen = true;
      break;
    }
    delay(100);
  }
  Motors::stop();
  
  if (!foundGreen) {
    // Try right
    Motors::turnRight(SPEED_TURN);
    for (int i = 0; i < 20; i++) {
      Sensors::readAll(data);
      if (data.detectedColor == COLOR_GREEN) {
        foundGreen = true;
        break;
      }
      delay(100);
    }
    Motors::stop();
  }
  
  Serial.print(F("    Found green: ")); Serial.println(foundGreen ? "YES" : "NO");
  TEST_ASSERT(foundGreen, "Should find and select green path");
}

void testSection1Sequence() {
  TEST_CASE("Full Section 1 sequence (abbreviated)");
  
  Serial.println(F("    This tests the complete Section 1 flow"));
  Serial.println(F("    Steps: Line follow -> Pickup -> Green path -> Blue zone -> Drop"));
  
  TestFramework::waitForInput(F("Set up full Section 1 test track"));
  
  // This is a guided test - each step is confirmed
  bool passed = true;
  
  // Step 1: Line following
  TestFramework::countdown(3, F("Step 1: Line following"));
  Serial.println(F("    Following black line for 3 seconds..."));
  
  SensorData data;
  unsigned long timer = millis();
  while (millis() - timer < 3000) {
    Sensors::readAll(data);
    Navigation::followBlackLine(data);
    delay(TIME_LOOP_DELAY);
  }
  Motors::stop();
  
  Serial.println(F("    Did robot follow the line?"));
  TestFramework::waitForInput(F("Confirm and continue"));
  
  // Step 2: Simulate pickup
  Serial.println(F("    Step 2: Pickup (simulated)"));
  Servos::pickup();
  TEST_ASSERT(Servos::isHolding(), "Should be holding");
  
  // Step 3: Check for blue detection
  Serial.println(F("    Step 3: Move robot over BLUE zone"));
  TestFramework::waitForInput(F("Position robot over blue"));
  
  Sensors::readAll(data);
  bool sawBlue = (data.detectedColor == COLOR_BLUE);
  Serial.print(F("    Detected color: ")); Serial.println(Sensors::colorName(data.detectedColor));
  
  // Step 4: Drop
  if (sawBlue) {
    Serial.println(F("    Step 4: Dropping at blue zone"));
    Servos::drop();
  }
  
  TEST_ASSERT(!Servos::isHolding(), "Should have dropped box");
  
  Serial.println(F("\n    Section 1 sequence test complete!"));
}

} // namespace Integration

} // namespace TestNavigation
