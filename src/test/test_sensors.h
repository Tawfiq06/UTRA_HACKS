/**
 * @file test_sensors.h
 * @brief Comprehensive sensor tests
 * @details Tests for color sensor, ultrasonic, and IR sensors
 */

#ifndef TEST_SENSORS_H
#define TEST_SENSORS_H

#include "test_framework.h"
#include "../core/sensors.h"

namespace TestSensors {

/**
 * @brief Run all sensor tests
 */
void runAll();

// ============================================================================
// ULTRASONIC SENSOR TESTS
// ============================================================================
namespace Ultrasonic {
  /** @brief Test basic distance reading */
  void testBasicReading();
  
  /** @brief Test no obstacle (far distance) */
  void testNoObstacle();
  
  /** @brief Test obstacle detection at threshold */
  void testObstacleAtThreshold();
  
  /** @brief Test very close object */
  void testVeryClose();
  
  /** @brief Test timeout handling */
  void testTimeout();
  
  /** @brief Test reading consistency */
  void testConsistency();
  
  void runAll();
}

// ============================================================================
// COLOR SENSOR TESTS
// ============================================================================
namespace ColorSensor {
  /** @brief Test black surface detection */
  void testBlackDetection();
  
  /** @brief Test white surface detection */
  void testWhiteDetection();
  
  /** @brief Test red surface detection */
  void testRedDetection();
  
  /** @brief Test green surface detection */
  void testGreenDetection();
  
  /** @brief Test blue surface detection */
  void testBlueDetection();
  
  /** @brief Test transition between colors */
  void testColorTransition();
  
  /** @brief Test color detection consistency */
  void testConsistency();
  
  /** @brief Test edge case: similar colors */
  void testSimilarColors();
  
  void runAll();
}

// ============================================================================
// IR SENSOR TESTS
// ============================================================================
namespace IRSensors {
  /** @brief Test both sensors on line */
  void testBothOnLine();
  
  /** @brief Test left sensor only on line */
  void testLeftOnLine();
  
  /** @brief Test right sensor only on line */
  void testRightOnLine();
  
  /** @brief Test both sensors off line */
  void testBothOffLine();
  
  /** @brief Test line intersection detection */
  void testIntersection();
  
  void runAll();
}

} // namespace TestSensors

#endif // TEST_SENSORS_H
