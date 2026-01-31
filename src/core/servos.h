/**
 * @file servos.h
 * @brief Servo control for claw mechanism
 * @details Controls base arm and clamp servos
 */

#ifndef SERVOS_H
#define SERVOS_H

#include <Arduino.h>
#include <Servo.h>
#include "../config/pins.h"
#include "../config/constants.h"

// ============================================================================
// CLAW STATE ENUMERATION
// ============================================================================
enum ClawState : uint8_t {
  CLAW_IDLE = 0,
  CLAW_READY,      // Arm down, clamp open
  CLAW_GRABBING,   // Arm down, clamp closing
  CLAW_CARRYING,   // Arm up, clamp closed
  CLAW_RELEASING   // Arm down, clamp opening
};

// ============================================================================
// SERVOS CLASS
// ============================================================================
class Servos {
public:
  /**
   * @brief Initialize servo motors
   * @note Must be called in setup()
   */
  static void init();
  
  /**
   * @brief Open the clamp
   * @note O(1) - single servo write
   */
  static void openClamp();
  
  /**
   * @brief Close the clamp
   * @note O(1) - single servo write
   */
  static void closeClamp();
  
  /**
   * @brief Raise the arm
   * @note O(1) - single servo write
   */
  static void armUp();
  
  /**
   * @brief Lower the arm
   * @note O(1) - single servo write
   */
  static void armDown();
  
  /**
   * @brief Set arm to carrying position
   * @note O(1) - single servo write
   */
  static void armCarry();
  
  /**
   * @brief Execute pickup sequence
   * @details Blocking function: arm down -> close clamp -> arm carry
   * @note O(1) - fixed sequence of operations
   */
  static void pickup();
  
  /**
   * @brief Execute drop sequence
   * @details Blocking function: arm down -> open clamp -> arm carry
   * @note O(1) - fixed sequence of operations
   */
  static void drop();
  
  /**
   * @brief Execute ball launch sequence
   * @details Blocking function: arm down -> quick forward push -> arm flick
   * @note O(1) - fixed sequence of operations
   */
  static void launch();
  
  /**
   * @brief Check if robot is holding an object
   * @return true if clamp is closed and arm is up
   */
  static bool isHolding();
  
  /**
   * @brief Get current clamp angle
   * @return Current clamp servo position (0-180)
   */
  static uint8_t getClampAngle();
  
  /**
   * @brief Get current arm angle
   * @return Current arm servo position (0-180)
   */
  static uint8_t getArmAngle();

private:
  static Servo baseServo;
  static Servo clampServo;
  static bool holding;
  static uint8_t currentClampAngle;
  static uint8_t currentArmAngle;
};

#endif // SERVOS_H
