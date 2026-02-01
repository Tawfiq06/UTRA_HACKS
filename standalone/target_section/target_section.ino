/**
 * ╔═══════════════════════════════════════════════════════════════════════════╗
 * ║                    SECTION 2: TARGET SHOOTING                             ║
 * ║                                                                           ║
 * ║  MISSION: Climb ramp → Navigate colored zones to center →                ║
 * ║           Find ball → Shoot ball → Return down ramp                      ║
 * ╚═══════════════════════════════════════════════════════════════════════════╝
 * 
 * THE TARGET LAYOUT:
 * ==================
 * The target is a bullseye with colored rings:
 * 
 *     ┌─────────────────────────────────┐
 *     │           BLUE (outer)          │
 *     │    ┌─────────────────────┐      │
 *     │    │      RED (middle)   │      │
 *     │    │   ┌─────────────┐   │      │
 *     │    │   │   GREEN     │   │      │
 *     │    │   │  ┌───────┐  │   │      │
 *     │    │   │  │ BLACK │  │   │      │  ← Ball is here
 *     │    │   │  │(center)│  │   │      │
 *     │    │   │  └───────┘  │   │      │
 *     │    │   └─────────────┘   │      │
 *     │    └─────────────────────┘      │
 *     └─────────────────────────────────┘
 * 
 * STATE FLOW:
 * ===========
 *   [CLIMB RAMP] → [ON TARGET] → [NAV BLUE] → [NAV RED] → [NAV GREEN]
 *                                                              ↓
 *   [COMPLETE] ← [RETURN] ← [SHOOT] ← [FIND BALL] ← [REACH CENTER]
 */

#include <Servo.h>

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                           PIN DEFINITIONS                                  ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

// --- COLOR SENSOR (TCS3200) ---
#define PIN_COLOR_S0      2
#define PIN_COLOR_S1      3
#define PIN_COLOR_S2      4
#define PIN_COLOR_S3      5
#define PIN_COLOR_OUT     6

// --- MOTOR DRIVER (L298N) ---
// Motor A = LEFT wheel, Motor B = RIGHT wheel
#define PIN_MOTOR_ENA     9    // LEFT motor speed (PWM)
#define PIN_MOTOR_IN1     8    // LEFT motor direction
#define PIN_MOTOR_IN2     7    // LEFT motor direction
#define PIN_MOTOR_ENB     10   // RIGHT motor speed (PWM)
#define PIN_MOTOR_IN3     11   // RIGHT motor direction
#define PIN_MOTOR_IN4     12   // RIGHT motor direction

// Speed compensation: Right motor is faster
#define SPEED_COMPENSATION  0.9

// --- SERVOS ---
#define PIN_SERVO_BASE    A5   // Arm servo (for ball launching)
#define PIN_SERVO_CLAMP   A4   // Not used in this section

// --- ULTRASONIC SENSOR ---
#define PIN_ULTRA_TRIG    A0
#define PIN_ULTRA_ECHO    A1

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                              CONSTANTS                                     ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

// Motor speeds
#define SPEED_SLOW        100
#define SPEED_NORMAL      150
#define SPEED_FAST        200   // For climbing ramp
#define SPEED_TURN        120

// Sensor thresholds
#define DIST_BALL         20    // Distance to detect ball (cm)
#define COLOR_FREQ_MAX    150
#define COLOR_FREQ_BLACK  200
#define COLOR_MARGIN      20

// Servo positions
#define SERVO_ARM_UP      90    // Arm raised
#define SERVO_ARM_DOWN    0     // Arm lowered (for shooting)

// Timing
#define TIME_TURN_90      500   // ms for 90° turn

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                              DATA TYPES                                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

enum Color { 
  COLOR_NONE, 
  COLOR_BLACK,   // Center of target
  COLOR_WHITE, 
  COLOR_RED,     // Middle ring
  COLOR_GREEN,   // Inner ring
  COLOR_BLUE     // Outer ring
};

enum State {
  STATE_CLIMB_RAMP,     // Going up the ramp
  STATE_ON_TARGET,      // Just arrived on target, detecting initial color
  STATE_NAV_BLUE,       // Navigating through blue zone
  STATE_NAV_RED,        // Navigating through red zone
  STATE_NAV_GREEN,      // Navigating through green zone
  STATE_REACH_CENTER,   // Arrived at black center
  STATE_FIND_BALL,      // Looking for the ball
  STATE_SHOOT,          // Launching the ball
  STATE_RETURN,         // Going back down the ramp
  STATE_COMPLETE        // Section done
};

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                           GLOBAL VARIABLES                                 ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

Servo baseServo, clampServo;
State currentState = STATE_CLIMB_RAMP;
uint32_t stateStartTime = 0;
int8_t searchDir = 1;      // Direction to search: 1=right, -1=left
uint8_t searchCount = 0;   // Counter for search pattern

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                          SENSOR FUNCTIONS                                  ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * Read distance from ultrasonic sensor (in cm)
 */
float readDistance() {
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRA_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  
  unsigned long duration = pulseIn(PIN_ULTRA_ECHO, HIGH, 25000);
  return duration == 0 ? 999.0 : (duration * 0.034) / 2.0;
}

/**
 * Read color from TCS3200 sensor
 */
Color readColor() {
  // Read RED
  digitalWrite(PIN_COLOR_S2, LOW);
  digitalWrite(PIN_COLOR_S3, LOW);
  delay(10);
  uint16_t r = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  // Read GREEN
  digitalWrite(PIN_COLOR_S2, HIGH);
  digitalWrite(PIN_COLOR_S3, HIGH);
  delay(10);
  uint16_t g = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  // Read BLUE
  digitalWrite(PIN_COLOR_S2, LOW);
  digitalWrite(PIN_COLOR_S3, HIGH);
  delay(10);
  uint16_t b = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  // Handle timeouts
  if (r == 0) r = 999;
  if (g == 0) g = 999;
  if (b == 0) b = 999;
  
  // Determine color (higher value = less of that color)
  if (r > COLOR_FREQ_BLACK && g > COLOR_FREQ_BLACK && b > COLOR_FREQ_BLACK) 
    return COLOR_BLACK;
  if (r < g - COLOR_MARGIN && r < b - COLOR_MARGIN && r < COLOR_FREQ_MAX) 
    return COLOR_RED;
  if (g < r - COLOR_MARGIN && g < b - COLOR_MARGIN && g < COLOR_FREQ_MAX) 
    return COLOR_GREEN;
  if (b < r - COLOR_MARGIN && b < g - COLOR_MARGIN && b < COLOR_FREQ_MAX) 
    return COLOR_BLUE;
  
  return COLOR_NONE;
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                          MOTOR FUNCTIONS                                   ║
// ║              Motor A = LEFT wheel, Motor B = RIGHT wheel                  ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

void stopMotors() {
  analogWrite(PIN_MOTOR_ENA, 0);
  analogWrite(PIN_MOTOR_ENB, 0);
}

void moveForward(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH);  // LEFT forward
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, HIGH);  // RIGHT forward
  digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed);
  analogWrite(PIN_MOTOR_ENB, (uint8_t)(speed * SPEED_COMPENSATION));
}

void turnLeft(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, LOW);   // LEFT backward
  digitalWrite(PIN_MOTOR_IN2, HIGH);
  digitalWrite(PIN_MOTOR_IN3, HIGH);  // RIGHT forward
  digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed);
  analogWrite(PIN_MOTOR_ENB, (uint8_t)(speed * SPEED_COMPENSATION));
}

void turnRight(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH);  // LEFT forward
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, LOW);   // RIGHT backward
  digitalWrite(PIN_MOTOR_IN4, HIGH);
  analogWrite(PIN_MOTOR_ENA, speed);
  analogWrite(PIN_MOTOR_ENB, (uint8_t)(speed * SPEED_COMPENSATION));
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                       NAVIGATION TO CENTER                                 ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * Navigate toward the center of the target using a search pattern.
 * Moves forward, then alternates turning left/right to find inner colors.
 */
void navigateToCenter() {
  moveForward(SPEED_SLOW);
  delay(100);
  
  searchCount++;
  if (searchCount > 5) {
    // Alternate search direction
    if (searchDir > 0) {
      turnRight(SPEED_TURN);
    } else {
      turnLeft(SPEED_TURN);
    }
    delay(200);
    stopMotors();
    searchDir *= -1;  // Flip direction
    searchCount = 0;
  }
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                           STATE MACHINE                                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

void transitionTo(State newState) {
  currentState = newState;
  stateStartTime = millis();
  searchCount = 0;
  Serial.print(F("STATE: "));
  Serial.println(newState);
}

void processState() {
  Color color = readColor();
  float dist = readDistance();
  
  switch (currentState) {
    
    // ─────────────────────────────────────────────────────────────────────────
    // CLIMB RAMP: Go up fast until we reach the target
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_CLIMB_RAMP:
      moveForward(SPEED_FAST);
      // Stop when we see a color (we're on the target)
      if (color != COLOR_NONE && color != COLOR_WHITE) {
        stopMotors();
        transitionTo(STATE_ON_TARGET);
      }
      // Timeout after 5 seconds
      if (millis() - stateStartTime > 5000) {
        stopMotors();
        transitionTo(STATE_ON_TARGET);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // ON TARGET: Figure out which zone we landed on
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_ON_TARGET:
      if (color == COLOR_BLUE)       transitionTo(STATE_NAV_BLUE);
      else if (color == COLOR_RED)   transitionTo(STATE_NAV_RED);
      else if (color == COLOR_GREEN) transitionTo(STATE_NAV_GREEN);
      else if (color == COLOR_BLACK) transitionTo(STATE_REACH_CENTER);
      else {
        // Can't determine color, move forward a bit
        moveForward(SPEED_SLOW);
        delay(200);
        stopMotors();
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // NAV BLUE: We're in the outer ring, navigate inward
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_NAV_BLUE:
      navigateToCenter();
      color = readColor();
      if (color == COLOR_RED)        transitionTo(STATE_NAV_RED);
      else if (color == COLOR_GREEN) transitionTo(STATE_NAV_GREEN);
      else if (color == COLOR_BLACK) transitionTo(STATE_REACH_CENTER);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // NAV RED: We're in the middle ring, navigate inward
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_NAV_RED:
      navigateToCenter();
      color = readColor();
      if (color == COLOR_GREEN)      transitionTo(STATE_NAV_GREEN);
      else if (color == COLOR_BLACK) transitionTo(STATE_REACH_CENTER);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // NAV GREEN: We're in the inner ring, navigate to center
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_NAV_GREEN:
      navigateToCenter();
      color = readColor();
      if (color == COLOR_BLACK) transitionTo(STATE_REACH_CENTER);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // REACH CENTER: We made it to the black center!
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_REACH_CENTER:
      stopMotors();
      Serial.println(F(">>> CENTER REACHED <<<"));
      delay(500);
      transitionTo(STATE_FIND_BALL);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // FIND BALL: Look for the ball using ultrasonic sensor
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_FIND_BALL:
      if (dist < DIST_BALL && dist > 0) {
        // Ball found!
        transitionTo(STATE_SHOOT);
      } else {
        // Keep searching
        moveForward(SPEED_SLOW);
        delay(200);
        stopMotors();
        // Timeout after 3 seconds
        if (millis() - stateStartTime > 3000) {
          transitionTo(STATE_SHOOT);
        }
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // SHOOT: Launch the ball forward!
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_SHOOT:
      Serial.println(F(">>> SHOOTING BALL <<<"));
      
      // Lower arm to push ball
      baseServo.write(SERVO_ARM_DOWN);
      delay(300);
      
      // Ram forward to launch
      moveForward(SPEED_FAST);
      delay(400);
      stopMotors();
      
      // Raise arm back up
      baseServo.write(SERVO_ARM_UP);
      delay(150);
      
      Serial.println(F(">>> BALL LAUNCHED <<<"));
      delay(1000);
      transitionTo(STATE_RETURN);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // RETURN: Turn around and go back down the ramp
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_RETURN:
      // Turn 180 degrees
      turnRight(SPEED_TURN);
      delay(TIME_TURN_90 * 2);
      stopMotors();
      
      // Drive down the ramp
      moveForward(SPEED_NORMAL);
      delay(4000);
      stopMotors();
      
      transitionTo(STATE_COMPLETE);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // COMPLETE: Section 2 done!
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_COMPLETE:
      stopMotors();
      Serial.println(F("\n============================="));
      Serial.println(F("   SECTION 2 COMPLETE!"));
      Serial.println(F("============================="));
      Serial.println(F("Now upload obstacle_section.ino"));
      while (true) delay(1000);
      break;
  }
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                          SETUP & MAIN LOOP                                 ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

void setup() {
  Serial.begin(9600);
  
  // Color sensor pins
  pinMode(PIN_COLOR_S0, OUTPUT);
  pinMode(PIN_COLOR_S1, OUTPUT);
  pinMode(PIN_COLOR_S2, OUTPUT);
  pinMode(PIN_COLOR_S3, OUTPUT);
  pinMode(PIN_COLOR_OUT, INPUT);
  digitalWrite(PIN_COLOR_S0, HIGH);
  digitalWrite(PIN_COLOR_S1, LOW);  // 20% frequency scaling
  
  // Ultrasonic pins
  pinMode(PIN_ULTRA_TRIG, OUTPUT);
  pinMode(PIN_ULTRA_ECHO, INPUT);
  
  // Motor pins
  pinMode(PIN_MOTOR_ENA, OUTPUT);
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  pinMode(PIN_MOTOR_ENB, OUTPUT);
  pinMode(PIN_MOTOR_IN3, OUTPUT);
  pinMode(PIN_MOTOR_IN4, OUTPUT);
  
  // Servos
  baseServo.attach(PIN_SERVO_BASE);
  clampServo.attach(PIN_SERVO_CLAMP);
  baseServo.write(SERVO_ARM_DOWN);
  
  stopMotors();
  delay(1000);
  
  Serial.println(F("============================="));
  Serial.println(F("  SECTION 2: TARGET SHOOTING"));
  Serial.println(F("============================="));
  Serial.println(F("Mission: Ramp -> Center -> Shoot -> Return"));
  Serial.println();
  
  transitionTo(STATE_CLIMB_RAMP);
}

void loop() {
  processState();
  delay(50);
}
