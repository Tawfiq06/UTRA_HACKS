/**
 * @file test_sensors.cpp
 * @brief Comprehensive sensor tests implementation
 */

#include "test_sensors.h"

namespace TestSensors {

void runAll() {
  Ultrasonic::runAll();
  ColorSensor::runAll();
  IRSensors::runAll();
}

// ============================================================================
// ULTRASONIC SENSOR TESTS
// ============================================================================
namespace Ultrasonic {

void runAll() {
  TEST_SECTION("Ultrasonic Sensor");
  
  testBasicReading();
  testNoObstacle();
  testObstacleAtThreshold();
  testVeryClose();
  testTimeout();
  testConsistency();
}

void testBasicReading() {
  TEST_CASE("Basic distance reading");
  
  float dist = Sensors::readDistance();
  
  // Distance should be positive or timeout value
  TEST_ASSERT(dist >= 0, "Distance should be non-negative");
  TEST_ASSERT_RANGE(dist, 0.0f, 999.0f, "Distance in valid range");
}

void testNoObstacle() {
  TEST_CASE("No obstacle (clear path)");
  TestFramework::waitForInput(F("Ensure clear path ahead (>50cm)"));
  
  float dist = Sensors::readDistance();
  
  TEST_ASSERT(dist > DIST_OBSTACLE, "Should detect no obstacle");
  TEST_ASSERT_RANGE(dist, 20.0f, 500.0f, "Distance reading reasonable");
}

void testObstacleAtThreshold() {
  TEST_CASE("Obstacle at threshold distance");
  TestFramework::waitForInput(F("Place object at ~15cm"));
  
  float dist = Sensors::readDistance();
  
  TEST_ASSERT_RANGE(dist, 10.0f, 20.0f, "Should detect object near threshold");
}

void testVeryClose() {
  TEST_CASE("Very close object (<5cm)");
  TestFramework::waitForInput(F("Place object very close (<5cm)"));
  
  float dist = Sensors::readDistance();
  
  // Note: HC-SR04 has minimum range of ~2cm
  TEST_ASSERT_RANGE(dist, 0.0f, 10.0f, "Should detect very close object");
}

void testTimeout() {
  TEST_CASE("Timeout handling (no echo)");
  TestFramework::waitForInput(F("Point sensor at open space/sky"));
  
  float dist = Sensors::readDistance();
  
  // Should return timeout value (999) or very large distance
  TEST_ASSERT(dist > 300 || dist == 999, "Should handle timeout gracefully");
}

void testConsistency() {
  TEST_CASE("Reading consistency");
  TestFramework::waitForInput(F("Keep object at fixed distance"));
  
  float readings[5];
  for (int i = 0; i < 5; i++) {
    readings[i] = Sensors::readDistance();
    delay(100);
  }
  
  // Calculate variance
  float sum = 0, avg;
  for (int i = 0; i < 5; i++) sum += readings[i];
  avg = sum / 5.0f;
  
  float variance = 0;
  for (int i = 0; i < 5; i++) {
    variance += (readings[i] - avg) * (readings[i] - avg);
  }
  variance /= 5.0f;
  
  // Standard deviation should be < 5cm for consistent readings
  TEST_ASSERT(sqrt(variance) < 5.0f, "Readings should be consistent (σ < 5cm)");
}

} // namespace Ultrasonic

// ============================================================================
// COLOR SENSOR TESTS
// ============================================================================
namespace ColorSensor {

void runAll() {
  TEST_SECTION("Color Sensor");
  
  testBlackDetection();
  testWhiteDetection();
  testRedDetection();
  testGreenDetection();
  testBlueDetection();
  testColorTransition();
  testConsistency();
  testSimilarColors();
}

void testBlackDetection() {
  TEST_CASE("Black surface detection");
  TestFramework::waitForInput(F("Place sensor over BLACK surface"));
  
  uint16_t r, g, b;
  Sensors::readColorRaw(r, g, b);
  Color detected = Sensors::detectColor(r, g, b);
  
  Serial.print(F("    RGB: ")); Serial.print(r); Serial.print(F(", "));
  Serial.print(g); Serial.print(F(", ")); Serial.println(b);
  
  TEST_ASSERT_EQ(COLOR_BLACK, detected, "Should detect BLACK");
  TEST_ASSERT(r > COLOR_FREQ_BLACK, "Red freq high for black");
  TEST_ASSERT(g > COLOR_FREQ_BLACK, "Green freq high for black");
  TEST_ASSERT(b > COLOR_FREQ_BLACK, "Blue freq high for black");
}

void testWhiteDetection() {
  TEST_CASE("White surface detection");
  TestFramework::waitForInput(F("Place sensor over WHITE surface"));
  
  uint16_t r, g, b;
  Sensors::readColorRaw(r, g, b);
  Color detected = Sensors::detectColor(r, g, b);
  
  Serial.print(F("    RGB: ")); Serial.print(r); Serial.print(F(", "));
  Serial.print(g); Serial.print(F(", ")); Serial.println(b);
  
  TEST_ASSERT_EQ(COLOR_WHITE, detected, "Should detect WHITE");
}

void testRedDetection() {
  TEST_CASE("Red surface detection");
  TestFramework::waitForInput(F("Place sensor over RED surface"));
  
  uint16_t r, g, b;
  Sensors::readColorRaw(r, g, b);
  Color detected = Sensors::detectColor(r, g, b);
  
  Serial.print(F("    RGB: ")); Serial.print(r); Serial.print(F(", "));
  Serial.print(g); Serial.print(F(", ")); Serial.println(b);
  
  TEST_ASSERT_EQ(COLOR_RED, detected, "Should detect RED");
  TEST_ASSERT(r < g, "Red freq lower than green");
  TEST_ASSERT(r < b, "Red freq lower than blue");
}

void testGreenDetection() {
  TEST_CASE("Green surface detection");
  TestFramework::waitForInput(F("Place sensor over GREEN surface"));
  
  uint16_t r, g, b;
  Sensors::readColorRaw(r, g, b);
  Color detected = Sensors::detectColor(r, g, b);
  
  Serial.print(F("    RGB: ")); Serial.print(r); Serial.print(F(", "));
  Serial.print(g); Serial.print(F(", ")); Serial.println(b);
  
  TEST_ASSERT_EQ(COLOR_GREEN, detected, "Should detect GREEN");
  TEST_ASSERT(g < r, "Green freq lower than red");
  TEST_ASSERT(g < b, "Green freq lower than blue");
}

void testBlueDetection() {
  TEST_CASE("Blue surface detection");
  TestFramework::waitForInput(F("Place sensor over BLUE surface"));
  
  uint16_t r, g, b;
  Sensors::readColorRaw(r, g, b);
  Color detected = Sensors::detectColor(r, g, b);
  
  Serial.print(F("    RGB: ")); Serial.print(r); Serial.print(F(", "));
  Serial.print(g); Serial.print(F(", ")); Serial.println(b);
  
  TEST_ASSERT_EQ(COLOR_BLUE, detected, "Should detect BLUE");
  TEST_ASSERT(b < r, "Blue freq lower than red");
  TEST_ASSERT(b < g, "Blue freq lower than green");
}

void testColorTransition() {
  TEST_CASE("Color transition detection");
  Serial.println(F("    Move sensor from BLACK to GREEN slowly..."));
  TestFramework::countdown(3, F("Starting"));
  
  Color lastColor = COLOR_NONE;
  bool transitionDetected = false;
  
  for (int i = 0; i < 30; i++) {
    uint16_t r, g, b;
    Sensors::readColorRaw(r, g, b);
    Color current = Sensors::detectColor(r, g, b);
    
    if (current != lastColor && lastColor != COLOR_NONE) {
      transitionDetected = true;
      Serial.print(F("    Transition: "));
      Serial.print(Sensors::colorName(lastColor));
      Serial.print(F(" -> "));
      Serial.println(Sensors::colorName(current));
    }
    lastColor = current;
    delay(100);
  }
  
  TEST_ASSERT(transitionDetected, "Should detect color transition");
}

void testConsistency() {
  TEST_CASE("Color reading consistency");
  TestFramework::waitForInput(F("Hold sensor steady over colored surface"));
  
  Color readings[10];
  for (int i = 0; i < 10; i++) {
    uint16_t r, g, b;
    Sensors::readColorRaw(r, g, b);
    readings[i] = Sensors::detectColor(r, g, b);
    delay(50);
  }
  
  // Count how many match the first reading
  int matches = 0;
  for (int i = 1; i < 10; i++) {
    if (readings[i] == readings[0]) matches++;
  }
  
  // At least 8/10 should match (80% consistency)
  TEST_ASSERT(matches >= 7, "At least 80% readings should match");
}

void testSimilarColors() {
  TEST_CASE("Similar color differentiation (edge case)");
  Serial.println(F("    This tests ability to differentiate similar colors"));
  
  TestFramework::waitForInput(F("Place sensor over DARK RED surface"));
  uint16_t r1, g1, b1;
  Sensors::readColorRaw(r1, g1, b1);
  Color darkRed = Sensors::detectColor(r1, g1, b1);
  
  TestFramework::waitForInput(F("Place sensor over BRIGHT RED surface"));
  uint16_t r2, g2, b2;
  Sensors::readColorRaw(r2, g2, b2);
  Color brightRed = Sensors::detectColor(r2, g2, b2);
  
  // Both should be detected as red
  TEST_ASSERT(darkRed == COLOR_RED || darkRed == COLOR_BLACK, 
              "Dark red should be RED or BLACK");
  TEST_ASSERT(brightRed == COLOR_RED, "Bright red should be RED");
}

} // namespace ColorSensor

// ============================================================================
// IR SENSOR TESTS
// ============================================================================
namespace IRSensors {

void runAll() {
  TEST_SECTION("IR Sensors (Line Following)");
  
  testBothOnLine();
  testLeftOnLine();
  testRightOnLine();
  testBothOffLine();
  testIntersection();
}

void testBothOnLine() {
  TEST_CASE("Both sensors on black line");
  TestFramework::waitForInput(F("Position BOTH sensors on black line"));
  
  bool left, right;
  Sensors::readIR(left, right);
  
  TEST_ASSERT(left, "Left sensor should detect line");
  TEST_ASSERT(right, "Right sensor should detect line");
}

void testLeftOnLine() {
  TEST_CASE("Only left sensor on line");
  TestFramework::waitForInput(F("Position LEFT sensor on line, RIGHT off"));
  
  bool left, right;
  Sensors::readIR(left, right);
  
  TEST_ASSERT(left, "Left sensor should detect line");
  TEST_ASSERT(!right, "Right sensor should NOT detect line");
}

void testRightOnLine() {
  TEST_CASE("Only right sensor on line");
  TestFramework::waitForInput(F("Position RIGHT sensor on line, LEFT off"));
  
  bool left, right;
  Sensors::readIR(left, right);
  
  TEST_ASSERT(!left, "Left sensor should NOT detect line");
  TEST_ASSERT(right, "Right sensor should detect line");
}

void testBothOffLine() {
  TEST_CASE("Both sensors off line");
  TestFramework::waitForInput(F("Position BOTH sensors on white/non-line surface"));
  
  bool left, right;
  Sensors::readIR(left, right);
  
  TEST_ASSERT(!left, "Left sensor should NOT detect line");
  TEST_ASSERT(!right, "Right sensor should NOT detect line");
}

void testIntersection() {
  TEST_CASE("Line intersection detection");
  Serial.println(F("    Move robot across a line intersection..."));
  TestFramework::countdown(3, F("Starting"));
  
  bool sawIntersection = false;
  
  for (int i = 0; i < 50; i++) {
    bool left, right;
    Sensors::readIR(left, right);
    
    // At intersection, both sensors might momentarily detect line
    // or we might see rapid transitions
    if (left && right) {
      sawIntersection = true;
      Serial.println(F("    Intersection detected!"));
      break;
    }
    delay(50);
  }
  
  TEST_ASSERT(sawIntersection, "Should detect intersection");
}

} // namespace IRSensors

} // namespace TestSensors
