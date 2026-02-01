# UTRA HACKS - Competition Robot

A three-section autonomous robot for the UTRA Hacks competition.

## Project Structure

```
standalone/
├── start_section/      ← Section 1: Start area
│   └── start_section.ino
├── target_section/     ← Section 2: Target shooting
│   └── target_section.ino
├── obstacle_section/   ← Section 3: Obstacle course
│   └── obstacle_section.ino
└── diagnostic/         ← Hardware testing tool
    └── diagnostic.ino
```

## Competition Sections

### Section 1: Start Section
**File:** `standalone/start_section/start_section.ino`

**Mission:** Follow black line → Pick up box → Take green path → Drop at blue zone

### Section 2: Target Shooting  
**File:** `standalone/target_section/target_section.ino`

**Mission:** Climb ramp → Navigate colored rings to center → Shoot ball → Return

### Section 3: Obstacle Course
**File:** `standalone/obstacle_section/obstacle_section.ino`

**Mission:** Follow red line → Pick up box → Avoid 2 obstacles → Drop at blue → Return home

## Hardware Wiring

### Motor Driver (L298N)
| Motor | Function | Arduino Pin |
|-------|----------|-------------|
| Motor A | LEFT wheel | ENA=9, IN1=8, IN2=7 |
| Motor B | RIGHT wheel | ENB=10, IN3=11, IN4=12 |

### Sensors
| Sensor | Pins |
|--------|------|
| Color (TCS3200) | S0=2, S1=3, S2=4, S3=5, OUT=6 |
| Ultrasonic (HC-SR04) | TRIG=A0, ECHO=A1 |
| IR Left | A2 |
| IR Right | A3 |

### Servos
| Servo | Pin |
|-------|-----|
| Arm (Base) | A5 |
| Claw (Clamp) | A4 |

## How to Use

1. Open Arduino IDE
2. Open the section you want to run (e.g., `standalone/start_section/start_section.ino`)
3. Select your board (Arduino UNO R4 Minima)
4. Select the correct COM port
5. Upload!

## Diagnostic Tool

Use `standalone/diagnostic/diagnostic.ino` to test individual components:

1. Upload the diagnostic sketch
2. Open Serial Monitor (9600 baud)
3. Use the menu to test motors, sensors, and servos

## Speed Compensation

The right motor runs faster than the left. A 0.9 multiplier is applied to the right motor speed to make the robot drive straight.

If your robot still drifts, adjust `SPEED_COMPENSATION` in the code:
- Drifts right? → Decrease value (e.g., 0.85)
- Drifts left? → Increase value (e.g., 0.95)
