/**
 * @file test_navigation.h
 * @brief Navigation behavior tests
 * @details Tests for line following, obstacle avoidance, and path planning
 */

#ifndef TEST_NAVIGATION_H
#define TEST_NAVIGATION_H

#include "test_framework.h"
#include "../core/navigation.h"
#include "../core/sensors.h"
#include "../core/motors.h"

namespace TestNavigation {

/**
 * @brief Run all navigation tests
 */
void runAll();

// ============================================================================
// LINE FOLLOWING TESTS
// ============================================================================
namespace LineFollowing {
  /** @brief Test straight black line following */
  void testStraightLine();
  
  /** @brief Test gentle curve following */
  void testGentleCurve();
  
  /** @brief Test sharp turn handling */
  void testSharpTurn();
  
  /** @brief Test line recovery when lost */
  void testLineRecovery();
  
  /** @brief Test colored line following */
  void testColorLine();
  
  void runAll();
}

// ============================================================================
// OBSTACLE AVOIDANCE TESTS
// ============================================================================
namespace ObstacleAvoidance {
  /** @brief Test obstacle detection */
  void testObstacleDetection();
  
  /** @brief Test avoidance maneuver */
  void testAvoidanceManeuver();
  
  /** @brief Test wall hugging */
  void testWallHugging();
  
  void runAll();
}

// ============================================================================
// INTEGRATION TESTS
// ============================================================================
namespace Integration {
  /** @brief Test pickup while following line */
  void testPickupOnLine();
  
  /** @brief Test color-based path selection */
  void testPathSelection();
  
  /** @brief Test full section 1 behavior */
  void testSection1Sequence();
  
  void runAll();
}

} // namespace TestNavigation

#endif // TEST_NAVIGATION_H
