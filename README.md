# UTRA HACKS Competition Robot

A modular, well-tested Arduino robot for the UTRA HACKS competition featuring line following, color detection, obstacle avoidance, and object manipulation.

## Competition Sections

| Section | Task | Upload Point |
|---------|------|--------------|
| **1. Start** | Follow black line → Pick up box → Take green path → Drop at blue zone | Competition start |
| **2. Target** | Climb ramp → Navigate colored rings to center → Shoot ball | 1st re-upload |
| **3. Obstacle** | Follow red line → Avoid obstacles → Drop box → Return to start | 2nd re-upload |

## Quick Start

### 1. Test Your Robot
```bash
# Upload the test runner
src/test/test_runner/test_runner.ino

# In Serial Monitor (9600 baud):
# Press 5 for Quick Diagnostics
# Press 6 for Calibration Mode
```

### 2. Competition Day
```bash
# At START:
src/sections/section1/section1.ino

# At 1st RE-UPLOAD:
src/sections/section2/section2.ino

# At 2nd RE-UPLOAD:
src/sections/section3/section3.ino
```

## Project Structure

```
UTRA_HACKS-main/
├── src/                       # Clean, modular code
│   ├── config/                # Pin & constant definitions
│   ├── core/                  # Reusable modules (sensors, motors, etc.)
│   ├── sections/              # Competition section code
│   └── test/                  # Comprehensive test suite
│
├── Archive/                   # Legacy code reference
└── README.md                  # This file
```

## Hardware Requirements

- Arduino Uno/Nano
- L298N Motor Driver
- TCS3200 Color Sensor
- HC-SR04 Ultrasonic Sensor
- 2x IR Line Sensors
- 2x SG90 Servos (arm + clamp)
- 2x DC Motors with wheels

## Wiring Diagram

```
┌─────────────────────────────────────────────────────────┐
│                      ARDUINO                             │
├─────────────────────────────────────────────────────────┤
│                                                          │
│  D2 ────── Color S0      A0 ────── Ultrasonic TRIG      │
│  D3 ────── Color S1      A1 ────── Ultrasonic ECHO      │
│  D4 ────── Color S2      A2 ────── IR Left              │
│  D5 ────── Color S3      A3 ────── IR Right             │
│  D6 ────── Color OUT     A4 ────── Servo Clamp          │
│                                                          │
│  D7 ────── Motor IN2     D11 ────── Motor IN3           │
│  D8 ────── Motor IN1     D12 ────── Servo Base          │
│  D9 ────── Motor ENA     D13 ────── Motor IN4           │
│  D10 ───── Motor ENB                                     │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

## Testing

The test framework includes 30+ test cases across three categories:

### Sensor Tests
- Ultrasonic distance accuracy
- Color detection (black, white, red, green, blue)
- IR line detection

### Motor Tests
- Forward/backward movement
- Turn accuracy
- Servo pickup/drop sequences

### Navigation Tests
- Line following
- Obstacle avoidance
- Color-based path selection

Run all tests:
```cpp
// Upload test_runner.ino
// Select option 4 in menu
```

## Complexity Analysis

| Operation | Time | Space |
|-----------|------|-------|
| Sensor read | O(1) | O(1) |
| Motor command | O(1) | O(1) |
| Line following | O(1) | O(1) |
| Obstacle avoidance | O(n)* | O(1) |
| Complete section | O(n) | O(1) |

*n = obstacle size

## Calibration

Edit `src/config/constants.h` to tune:

```cpp
// Motor speeds (0-255)
#define SPEED_NORMAL      150

// Color thresholds
#define COLOR_FREQ_BLACK  200

// Distance thresholds (cm)
#define DIST_OBSTACLE     15

// Timing (ms)
#define TIME_TURN_90      500
```

## Team

UTRA HACKS 2026

## License

MIT
