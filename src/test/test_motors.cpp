/**
 * @file test_motors.cpp
 * @brief Motor and servo tests implementation
 */

#include "test_motors.h"

namespace TestMotors {

void runAll() {
  Drive::runAll();
  Servo::runAll();
}

// ============================================================================
// DRIVE MOTOR TESTS
// ============================================================================
namespace Drive {

void runAll() {
  TEST_SECTION("Drive Motors");
  
  Serial.println(F("\n  ⚠️  WARNING: Robot will move during these tests!"));
  Serial.println(F("  Place robot on blocks or hold it securely."));
  TestFramework::waitForInput(F("Ready to test motors?"));
  
  testStop();
  testForward();
  testBackward();
  testTurnLeft();
  testTurnRight();
  testCurveLeft();
  testCurveRight();
  testSpeedControl();
  testTurnAccuracy();
}

void testStop() {
  TEST_CASE("Motor stop");
  
  Motors::forward(SPEED_NORMAL);
  delay(500);
  Motors::stop();
  delay(200);
  
  // Visual confirmation
  Serial.println(F("    Did motors stop completely? (Check visually)"));
  TEST_ASSERT(true, "Stop command executed");
}

void testForward() {
  TEST_CASE("Forward movement");
  TestFramework::countdown(2, F("Moving forward"));
  
  Motors::forward(SPEED_NORMAL);
  delay(1000);
  Motors::stop();
  
  Serial.println(F("    Did robot move FORWARD? (Check visually)"));
  TEST_ASSERT(true, "Forward movement executed");
}

void testBackward() {
  TEST_CASE("Backward movement");
  TestFramework::countdown(2, F("Moving backward"));
  
  Motors::backward(SPEED_NORMAL);
  delay(1000);
  Motors::stop();
  
  Serial.println(F("    Did robot move BACKWARD? (Check visually)"));
  TEST_ASSERT(true, "Backward movement executed");
}

void testTurnLeft() {
  TEST_CASE("Turn left (pivot)");
  TestFramework::countdown(2, F("Turning left"));
  
  Motors::turnLeft(SPEED_TURN);
  delay(TIME_TURN_90);
  Motors::stop();
  
  Serial.println(F("    Did robot turn LEFT? (Check visually)"));
  TEST_ASSERT(true, "Left turn executed");
}

void testTurnRight() {
  TEST_CASE("Turn right (pivot)");
  TestFramework::countdown(2, F("Turning right"));
  
  Motors::turnRight(SPEED_TURN);
  delay(TIME_TURN_90);
  Motors::stop();
  
  Serial.println(F("    Did robot turn RIGHT? (Check visually)"));
  TEST_ASSERT(true, "Right turn executed");
}

void testCurveLeft() {
  TEST_CASE("Curve left");
  TestFramework::countdown(2, F("Curving left"));
  
  Motors::curveLeft(SPEED_NORMAL);
  delay(1000);
  Motors::stop();
  
  Serial.println(F("    Did robot curve LEFT while moving forward?"));
  TEST_ASSERT(true, "Left curve executed");
}

void testCurveRight() {
  TEST_CASE("Curve right");
  TestFramework::countdown(2, F("Curving right"));
  
  Motors::curveRight(SPEED_NORMAL);
  delay(1000);
  Motors::stop();
  
  Serial.println(F("    Did robot curve RIGHT while moving forward?"));
  TEST_ASSERT(true, "Right curve executed");
}

void testSpeedControl() {
  TEST_CASE("Speed control");
  Serial.println(F("    Testing different speeds: SLOW -> NORMAL -> FAST"));
  
  TestFramework::countdown(2, F("Starting speed test"));
  
  // Slow
  Serial.println(F("    Speed: SLOW"));
  Motors::forward(SPEED_SLOW);
  delay(800);
  
  // Normal
  Serial.println(F("    Speed: NORMAL"));
  Motors::forward(SPEED_NORMAL);
  delay(800);
  
  // Fast
  Serial.println(F("    Speed: FAST"));
  Motors::forward(SPEED_FAST);
  delay(800);
  
  Motors::stop();
  
  Serial.println(F("    Did speed increase visibly?"));
  TEST_ASSERT(true, "Speed control executed");
}

void testTurnAccuracy() {
  TEST_CASE("Turn accuracy (90 degrees)");
  Serial.println(F("    This tests if TIME_TURN_90 is calibrated correctly"));
  
  TestFramework::waitForInput(F("Mark robot's starting direction"));
  TestFramework::countdown(2, F("Executing 90° right turn"));
  
  Motors::turnRight(SPEED_TURN);
  delay(TIME_TURN_90);
  Motors::stop();
  
  Serial.println(F("    Did robot turn approximately 90 degrees?"));
  Serial.println(F("    If not, adjust TIME_TURN_90 in constants.h"));
  TEST_ASSERT(true, "Turn accuracy test executed");
}

} // namespace Drive

// ============================================================================
// SERVO TESTS
// ============================================================================
namespace Servo {

void runAll() {
  TEST_SECTION("Servos (Claw Mechanism)");
  
  Serial.println(F("\n  ⚠️  NOTE: Servos will move during these tests!"));
  Serial.println(F("  Ensure claw has clearance to move freely."));
  TestFramework::waitForInput(F("Ready to test servos?"));
  
  testClampOpen();
  testClampClose();
  testArmUp();
  testArmDown();
  testArmCarry();
  testPickupSequence();
  testDropSequence();
  testLaunchSequence();
  testRapidMovements();
}

void testClampOpen() {
  TEST_CASE("Clamp open");
  
  Servos::openClamp();
  
  TEST_ASSERT_EQ((uint8_t)SERVO_CLAMP_OPEN, Servos::getClampAngle(), 
                 "Clamp angle should be OPEN");
  Serial.println(F("    Is clamp OPEN? (Check visually)"));
}

void testClampClose() {
  TEST_CASE("Clamp close");
  
  Servos::closeClamp();
  
  TEST_ASSERT_EQ((uint8_t)SERVO_CLAMP_CLOSED, Servos::getClampAngle(),
                 "Clamp angle should be CLOSED");
  Serial.println(F("    Is clamp CLOSED? (Check visually)"));
  
  // Return to open
  Servos::openClamp();
}

void testArmUp() {
  TEST_CASE("Arm up");
  
  Servos::armUp();
  
  TEST_ASSERT_EQ((uint8_t)SERVO_ARM_UP, Servos::getArmAngle(),
                 "Arm angle should be UP");
  Serial.println(F("    Is arm UP? (Check visually)"));
}

void testArmDown() {
  TEST_CASE("Arm down");
  
  Servos::armDown();
  
  TEST_ASSERT_EQ((uint8_t)SERVO_ARM_DOWN, Servos::getArmAngle(),
                 "Arm angle should be DOWN");
  Serial.println(F("    Is arm DOWN? (Check visually)"));
}

void testArmCarry() {
  TEST_CASE("Arm carry position");
  
  Servos::armCarry();
  
  TEST_ASSERT_EQ((uint8_t)SERVO_ARM_CARRY, Servos::getArmAngle(),
                 "Arm angle should be CARRY");
  Serial.println(F("    Is arm at CARRY position (45°)? (Check visually)"));
}

void testPickupSequence() {
  TEST_CASE("Pickup sequence");
  
  TestFramework::waitForInput(F("Place test object in front of claw"));
  TestFramework::countdown(2, F("Executing pickup"));
  
  Servos::pickup();
  
  TEST_ASSERT(Servos::isHolding(), "Should be holding after pickup");
  Serial.println(F("    Did claw pick up the object?"));
}

void testDropSequence() {
  TEST_CASE("Drop sequence");
  
  TestFramework::countdown(2, F("Executing drop"));
  
  Servos::drop();
  
  TEST_ASSERT(!Servos::isHolding(), "Should not be holding after drop");
  Serial.println(F("    Did claw release the object?"));
}

void testLaunchSequence() {
  TEST_CASE("Launch sequence (for ball shooting)");
  
  TestFramework::waitForInput(F("Place ball in front of arm"));
  TestFramework::countdown(2, F("Executing launch"));
  
  Servos::launch();
  
  Serial.println(F("    Did arm perform launch motion?"));
  TEST_ASSERT(true, "Launch sequence executed");
  
  // Return to carry position
  Servos::armCarry();
}

void testRapidMovements() {
  TEST_CASE("Rapid servo movements (stress test)");
  Serial.println(F("    Testing rapid open/close cycles..."));
  
  for (int i = 0; i < 5; i++) {
    Servos::closeClamp();
    delay(100);
    Servos::openClamp();
    delay(100);
  }
  
  TEST_ASSERT(Servos::getClampAngle() == SERVO_CLAMP_OPEN, 
              "Clamp should end in OPEN position");
  Serial.println(F("    Did clamp cycle 5 times without issues?"));
}

} // namespace Servo

} // namespace TestMotors
