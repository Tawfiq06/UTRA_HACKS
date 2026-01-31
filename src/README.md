# UTRA HACKS Robot - Clean Architecture

## Project Structure

```
src/
├── config/                    # Configuration files
│   ├── pins.h                 # Pin assignments (centralized)
│   └── constants.h            # Tunable parameters
│
├── core/                      # Core modules
│   ├── sensors.h/cpp          # Sensor abstraction layer
│   ├── motors.h/cpp           # Motor control
│   ├── servos.h/cpp           # Servo control
│   └── navigation.h/cpp       # Navigation behaviors
│
├── sections/                  # Competition sections
│   ├── section1/              # Start section
│   ├── section2/              # Target shooting
│   └── section3/              # Obstacle course
│
└── test/                      # Test framework
    ├── test_framework.h/cpp   # Test utilities
    ├── test_sensors.h/cpp     # Sensor tests
    ├── test_motors.h/cpp      # Motor tests
    ├── test_navigation.h/cpp  # Navigation tests
    └── test_runner/           # Main test executable
```

## Quick Start

### 1. Run Tests First
```
Upload: src/test/test_runner/test_runner.ino
Open Serial Monitor at 9600 baud
Select option 5 for Quick Diagnostics
```

### 2. Competition Day
```
START:           Upload src/sections/section1/section1.ino
RE-UPLOAD 1:     Upload src/sections/section2/section2.ino
RE-UPLOAD 2:     Upload src/sections/section3/section3.ino
```

## Test Cases Summary

### Happy Path Tests
| Test | Description | Expected Result |
|------|-------------|-----------------|
| Straight line follow | Robot follows straight black line | Stays on line >90% |
| Color detection | Detects R/G/B/Black surfaces | Correct color ID |
| Box pickup | Approaches and grabs box | Holding = true |
| Obstacle avoidance | Navigates around obstacle | Clears without collision |
| Target navigation | Moves from outer to inner ring | Reaches black center |

### Edge Case Tests
| Test | Description | Expected Result |
|------|-------------|-----------------|
| Line recovery | Finds line when started off-line | Recovers within 5s |
| Sharp turn | Handles 90° turns | Completes turn |
| Similar colors | Differentiates dark red from bright red | Correct classification |
| Timeout handling | Sensor returns no data | Graceful fallback |
| Rapid servo | Multiple quick open/close cycles | No servo damage |

### Error Case Tests
| Test | Description | Expected Result |
|------|-------------|-----------------|
| No echo | Ultrasonic timeout | Returns 999, no crash |
| Lost line | Both IR sensors off line | Enters search mode |
| No color | Sensor over unknown surface | Returns COLOR_NONE |
| Obstacle too close | Distance < 5cm | Emergency stop |

## Time & Space Complexity

### Core Functions
| Function | Time | Space | Notes |
|----------|------|-------|-------|
| `Sensors::readAll()` | O(1) | O(1) | Fixed sensor reads |
| `Motors::forward()` | O(1) | O(1) | Direct PWM write |
| `Navigation::followBlackLine()` | O(1) | O(1) | State-based decision |
| `Navigation::avoidObstacleRight()` | O(n) | O(1) | n = obstacle size |
| `Servos::pickup()` | O(1) | O(1) | Fixed sequence |

### Main Loop
- **Time**: O(1) per iteration
- **Space**: O(1) total
- **Loop Rate**: ~20Hz (50ms delay)

## Hardware Wiring

### Pin Map
```
Pin 2  -> Color S0
Pin 3  -> Color S1
Pin 4  -> Color S2
Pin 5  -> Color S3
Pin 6  -> Color OUT
Pin 7  -> Motor IN2
Pin 8  -> Motor IN1
Pin 9  -> Motor ENA (PWM)
Pin 10 -> Motor ENB (PWM)
Pin 11 -> Motor IN3
Pin 12 -> Servo Base
Pin 13 -> Motor IN4
Pin A0 -> Ultrasonic TRIG
Pin A1 -> Ultrasonic ECHO
Pin A2 -> IR Left
Pin A3 -> IR Right
Pin A4 -> Servo Clamp
```

## Calibration Guide

### 1. Motor Turn Calibration
```cpp
// In constants.h, adjust:
#define TIME_TURN_90  500  // Increase if turns are too short
```

### 2. Color Sensor Calibration
```cpp
// Run calibration mode (option 6 in test runner)
// Note the RGB values for each color
// Adjust in constants.h:
#define COLOR_FREQ_MAX    150  // Max freq for valid color
#define COLOR_FREQ_BLACK  200  // Threshold for black
#define COLOR_MARGIN      20   // Difference between colors
```

### 3. Distance Calibration
```cpp
// In constants.h, adjust:
#define DIST_OBSTACLE     15  // Detection range (cm)
#define DIST_BOX_PICKUP   5   // Pickup distance (cm)
```

## Troubleshooting

### Motors don't move
1. Check motor driver power (12V)
2. Verify IN1-IN4 pin connections
3. Check ENA/ENB PWM connections

### Color sensor wrong readings
1. Verify 5V power to sensor
2. Check sensor height (5-10mm from surface)
3. Run calibration mode and adjust thresholds

### Robot veers off line
1. Check IR sensor alignment
2. Adjust SPEED_NORMAL down
3. Check for wheel slippage

### Servo jitters
1. Check servo power (5V, adequate current)
2. Add delay between movements
3. Verify single servo write per position

## Competition Checklist

- [ ] Battery fully charged
- [ ] All connections secure
- [ ] Quick diagnostics pass
- [ ] Color thresholds calibrated for arena
- [ ] Turn timing verified
- [ ] Claw picks up test box
- [ ] Section files ready to upload

Good luck!
