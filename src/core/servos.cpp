/**
 * @file servos.cpp
 * @brief Servo control implementation
 */

#include "servos.h"

// Static member initialization
Servo Servos::baseServo;
Servo Servos::clampServo;
bool Servos::holding = false;
uint8_t Servos::currentClampAngle = SERVO_CLAMP_OPEN;
uint8_t Servos::currentArmAngle = SERVO_ARM_DOWN;

// ============================================================================
// INITIALIZATION
// ============================================================================
void Servos::init() {
  baseServo.attach(PIN_SERVO_BASE);
  clampServo.attach(PIN_SERVO_CLAMP);
  
  // Set initial position: arm down, clamp open
  baseServo.write(SERVO_ARM_DOWN);
  clampServo.write(SERVO_CLAMP_OPEN);
  
  currentArmAngle = SERVO_ARM_DOWN;
  currentClampAngle = SERVO_CLAMP_OPEN;
  holding = false;
  
  delay(TIME_SERVO_MOVE);
}

// ============================================================================
// CLAMP CONTROL - O(1)
// ============================================================================
void Servos::openClamp() {
  clampServo.write(SERVO_CLAMP_OPEN);
  currentClampAngle = SERVO_CLAMP_OPEN;
  delay(TIME_SERVO_MOVE);
}

void Servos::closeClamp() {
  clampServo.write(SERVO_CLAMP_CLOSED);
  currentClampAngle = SERVO_CLAMP_CLOSED;
  delay(TIME_SERVO_MOVE);
}

// ============================================================================
// ARM CONTROL - O(1)
// ============================================================================
void Servos::armUp() {
  baseServo.write(SERVO_ARM_UP);
  currentArmAngle = SERVO_ARM_UP;
  delay(TIME_SERVO_MOVE + 200);  // Extra time for larger movement
}

void Servos::armDown() {
  baseServo.write(SERVO_ARM_DOWN);
  currentArmAngle = SERVO_ARM_DOWN;
  delay(TIME_SERVO_MOVE + 200);
}

void Servos::armCarry() {
  baseServo.write(SERVO_ARM_CARRY);
  currentArmAngle = SERVO_ARM_CARRY;
  delay(TIME_SERVO_MOVE);
}

// ============================================================================
// COMPOUND MOVEMENTS - O(1)
// ============================================================================
void Servos::pickup() {
  // Sequence: lower arm -> close clamp -> raise to carry position
  armDown();
  delay(100);
  closeClamp();
  delay(100);
  armCarry();
  holding = true;
}

void Servos::drop() {
  // Sequence: lower arm -> open clamp -> raise to carry position
  armDown();
  delay(100);
  openClamp();
  delay(100);
  armCarry();
  holding = false;
}

void Servos::launch() {
  // Sequence: lower arm -> position for push
  armDown();
  delay(100);
  
  // Quick upward flick to launch
  baseServo.write(SERVO_ARM_UP);
  delay(100);  // Quick motion
  
  currentArmAngle = SERVO_ARM_UP;
}

// ============================================================================
// STATE QUERIES - O(1)
// ============================================================================
bool Servos::isHolding() {
  return holding;
}

uint8_t Servos::getClampAngle() {
  return currentClampAngle;
}

uint8_t Servos::getArmAngle() {
  return currentArmAngle;
}
