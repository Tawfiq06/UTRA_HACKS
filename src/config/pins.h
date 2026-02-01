/**
 * @file pins.h
 * @brief Pin configuration for UTRA HACKS robot
 * @details Centralized pin definitions to avoid conflicts and enable easy hardware changes
 */

#ifndef PINS_H
#define PINS_H

// ============================================================================
// COLOR SENSOR (TCS3200)
// ============================================================================
#define PIN_COLOR_S0      2   // Frequency scaling select bit 0
#define PIN_COLOR_S1      3   // Frequency scaling select bit 1
#define PIN_COLOR_S2      4   // Photodiode select bit 0
#define PIN_COLOR_S3      5   // Photodiode select bit 1
#define PIN_COLOR_OUT     6   // Frequency output

// ============================================================================
// MOTOR DRIVER (L298N)
// ============================================================================
#define PIN_MOTOR_ENA     9   // Motor A PWM (Right motor)
#define PIN_MOTOR_IN1     8   // Motor A direction 1
#define PIN_MOTOR_IN2     7   // Motor A direction 2
#define PIN_MOTOR_ENB     10  // Motor B PWM (Left motor)
#define PIN_MOTOR_IN3     11  // Motor B direction 1
#define PIN_MOTOR_IN4     A5  // Motor B direction 2 (Changed from 13 for R4)

// ============================================================================
// SERVOS
// ============================================================================
#define PIN_SERVO_BASE    12  // Base servo (arm up/down)
#define PIN_SERVO_CLAMP   A4  // Clamp servo (open/close)

// ============================================================================
// ULTRASONIC SENSOR (HC-SR04)
// ============================================================================
#define PIN_ULTRA_TRIG    A0  // Trigger pin
#define PIN_ULTRA_ECHO    A1  // Echo pin

// ============================================================================
// IR SENSORS (Line Following)
// ============================================================================
#define PIN_IR_LEFT       A2  // Left IR sensor
#define PIN_IR_RIGHT      A3  // Right IR sensor

#endif // PINS_H
