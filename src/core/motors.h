/**
 * @file motors.h
 * @brief Motor control abstraction layer
 * @details Provides clean interface for differential drive robot
 */

#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include "../config/pins.h"
#include "../config/constants.h"

// ============================================================================
// MOTOR DIRECTION ENUMERATION
// ============================================================================
enum MotorDirection : uint8_t {
  DIR_STOP = 0,
  DIR_FORWARD,
  DIR_BACKWARD
};

// ============================================================================
// MOTORS CLASS
// ============================================================================
class Motors {
public:
  /**
   * @brief Initialize motor pins
   * @note Must be called in setup()
   */
  static void init();
  
  /**
   * @brief Stop all motors immediately
   * @note O(1) - 6 digital writes
   */
  static void stop();
  
  /**
   * @brief Move robot forward
   * @param speed PWM value (0-255)
   * @note O(1) - 6 digital/analog writes
   */
  static void forward(uint8_t speed = SPEED_NORMAL);
  
  /**
   * @brief Move robot backward
   * @param speed PWM value (0-255)
   * @note O(1) - 6 digital/analog writes
   */
  static void backward(uint8_t speed = SPEED_NORMAL);
  
  /**
   * @brief Turn robot left (pivot)
   * @param speed PWM value (0-255)
   * @note O(1) - Left motor backward, right motor forward
   */
  static void turnLeft(uint8_t speed = SPEED_TURN);
  
  /**
   * @brief Turn robot right (pivot)
   * @param speed PWM value (0-255)
   * @note O(1) - Left motor forward, right motor backward
   */
  static void turnRight(uint8_t speed = SPEED_TURN);
  
  /**
   * @brief Gentle curve left while moving forward
   * @param speed Base PWM value (0-255)
   * @note O(1) - Right motor at full speed, left at half
   */
  static void curveLeft(uint8_t speed = SPEED_NORMAL);
  
  /**
   * @brief Gentle curve right while moving forward
   * @param speed Base PWM value (0-255)
   * @note O(1) - Left motor at full speed, right at half
   */
  static void curveRight(uint8_t speed = SPEED_NORMAL);
  
  /**
   * @brief Set individual motor speeds and directions
   * @param leftSpeed Left motor PWM (0-255)
   * @param leftDir Left motor direction
   * @param rightSpeed Right motor PWM (0-255)
   * @param rightDir Right motor direction
   * @note O(1) - 6 digital/analog writes
   */
  static void set(uint8_t leftSpeed, MotorDirection leftDir,
                  uint8_t rightSpeed, MotorDirection rightDir);

private:
  /**
   * @brief Set single motor state
   * @param in1 First direction pin
   * @param in2 Second direction pin
   * @param en Enable/PWM pin
   * @param speed PWM value
   * @param dir Direction
   */
  static void setMotor(uint8_t in1, uint8_t in2, uint8_t en, 
                       uint8_t speed, MotorDirection dir);
};

#endif // MOTORS_H
