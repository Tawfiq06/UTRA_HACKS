/**
 * @file test_motors.h
 * @brief Motor and servo tests
 * @details Tests for motor control and servo mechanisms
 */

#ifndef TEST_MOTORS_H
#define TEST_MOTORS_H

#include "test_framework.h"
#include "../core/motors.h"
#include "../core/servos.h"

namespace TestMotors {

/**
 * @brief Run all motor and servo tests
 */
void runAll();

// ============================================================================
// DRIVE MOTOR TESTS
// ============================================================================
namespace Drive {
  /** @brief Test forward movement */
  void testForward();
  
  /** @brief Test backward movement */
  void testBackward();
  
  /** @brief Test left turn */
  void testTurnLeft();
  
  /** @brief Test right turn */
  void testTurnRight();
  
  /** @brief Test stop function */
  void testStop();
  
  /** @brief Test curve left */
  void testCurveLeft();
  
  /** @brief Test curve right */
  void testCurveRight();
  
  /** @brief Test speed control */
  void testSpeedControl();
  
  /** @brief Test turn accuracy */
  void testTurnAccuracy();
  
  void runAll();
}

// ============================================================================
// SERVO TESTS
// ============================================================================
namespace Servo {
  /** @brief Test clamp open */
  void testClampOpen();
  
  /** @brief Test clamp close */
  void testClampClose();
  
  /** @brief Test arm up */
  void testArmUp();
  
  /** @brief Test arm down */
  void testArmDown();
  
  /** @brief Test arm carry position */
  void testArmCarry();
  
  /** @brief Test pickup sequence */
  void testPickupSequence();
  
  /** @brief Test drop sequence */
  void testDropSequence();
  
  /** @brief Test launch sequence */
  void testLaunchSequence();
  
  /** @brief Test rapid movements */
  void testRapidMovements();
  
  void runAll();
}

} // namespace TestMotors

#endif // TEST_MOTORS_H
