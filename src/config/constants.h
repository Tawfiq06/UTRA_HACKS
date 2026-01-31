/**
 * @file constants.h
 * @brief Tunable constants for UTRA HACKS robot
 * @details All configurable parameters in one place for easy calibration
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

// ============================================================================
// MOTOR SPEEDS (0-255 PWM)
// ============================================================================
#define SPEED_STOP        0
#define SPEED_SLOW        100   // Precision movements
#define SPEED_NORMAL      150   // Standard operation
#define SPEED_FAST        200   // Quick movements
#define SPEED_MAX         255   // Maximum speed
#define SPEED_TURN        120   // Turning speed

// ============================================================================
// DISTANCE THRESHOLDS (centimeters)
// ============================================================================
#define DIST_OBSTACLE     15    // Obstacle detection range
#define DIST_WALL_HUG     10    // Wall following distance
#define DIST_BOX_PICKUP   5     // Distance to pickup box
#define DIST_BALL_DETECT  20    // Ball detection range
#define DIST_MAX_VALID    400   // Maximum valid reading

// ============================================================================
// COLOR SENSOR THRESHOLDS
// Lower frequency = stronger color detection
// ============================================================================
#define COLOR_FREQ_MAX    150   // Maximum freq to consider valid color
#define COLOR_FREQ_BLACK  200   // All above this = black surface
#define COLOR_FREQ_WHITE  50    // All below this = white surface
#define COLOR_MARGIN      20    // Minimum difference between colors

// ============================================================================
// SERVO ANGLES (degrees)
// ============================================================================
#define SERVO_CLAMP_OPEN    90  // Clamp fully open
#define SERVO_CLAMP_CLOSED  0   // Clamp fully closed
#define SERVO_ARM_UP        90  // Arm raised
#define SERVO_ARM_DOWN      0   // Arm lowered
#define SERVO_ARM_CARRY     45  // Arm in carrying position

// ============================================================================
// TIMING CONSTANTS (milliseconds)
// ============================================================================
#define TIME_TURN_90      500   // Time for 90-degree turn
#define TIME_TURN_45      250   // Time for 45-degree turn
#define TIME_TURN_180     1000  // Time for 180-degree turn
#define TIME_SERVO_MOVE   300   // Servo movement delay
#define TIME_SENSOR_READ  10    // Color sensor read delay
#define TIME_LOOP_DELAY   50    // Main loop delay
#define TIME_DEBOUNCE     50    // Sensor debounce

// ============================================================================
// ULTRASONIC CONSTANTS
// ============================================================================
#define ULTRA_TIMEOUT_US  25000 // Timeout in microseconds
#define ULTRA_SPEED_CM    0.034 // Speed of sound cm/us (÷2 for round trip)

// ============================================================================
// IR SENSOR VALUES
// ============================================================================
#define IR_ON_LINE        LOW   // Value when sensor detects line
#define IR_OFF_LINE       HIGH  // Value when sensor off line

// ============================================================================
// STATE MACHINE TIMEOUTS (milliseconds)
// ============================================================================
#define TIMEOUT_RAMP      5000  // Max time climbing ramp
#define TIMEOUT_SEARCH    3000  // Max time searching for ball
#define TIMEOUT_RETURN    5000  // Max time returning to start

#endif // CONSTANTS_H
