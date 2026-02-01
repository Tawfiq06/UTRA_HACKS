/**
 * ╔═══════════════════════════════════════════════════════════════════════════╗
 * ║                    SECTION 3: OBSTACLE COURSE                             ║
 * ║                                                                           ║
 * ║  MISSION: Follow red line → Pick up box → Avoid 2 obstacles →            ║
 * ║           Drop box at blue → Return to start                             ║
 * ╚═══════════════════════════════════════════════════════════════════════════╝
 * 
 * THE COURSE LAYOUT:
 * ==================
 * 
 *   [START] ─── BLACK LINE ─── [INTERSECTION]
 *                                    │
 *                               RED LINE (left)
 *                                    │
 *                                  [BOX]
 *                                    │
 *                              [OBSTACLE 1]
 *                                    │
 *                              [OBSTACLE 2]
 *                                    │
 *                               [BLUE ZONE] ← Drop box here
 *                                    │
 *                              [INTERSECTION]
 *                                    │
 *                               BLACK LINE
 *                                    │
 *                                [START]
 * 
 * STATE FLOW:
 * ===========
 *   [FIND RED] → [FOLLOW RED] → [APPROACH BOX] → [PICKUP]
 *                                                    ↓
 *   [COMPLETE] ← [RETURN HOME] ← [DROP] ← [AVOID x2] ← [TO OBSTACLES]
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
#define PIN_SERVO_BASE    A5   // Arm servo (up/down)
#define PIN_SERVO_CLAMP   A4   // Claw servo (open/close)

// --- ULTRASONIC SENSOR ---
#define PIN_ULTRA_TRIG    A0
#define PIN_ULTRA_ECHO    A1

// --- IR LINE SENSORS ---
#define PIN_IR_LEFT       A2
#define PIN_IR_RIGHT      A3

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                              CONSTANTS                                     ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

// Motor speeds
#define SPEED_SLOW        100
#define SPEED_NORMAL      150
#define SPEED_TURN        120

// Distance thresholds (cm)
#define DIST_OBSTACLE     15   // Detect obstacle ahead
#define DIST_WALL_HUG     10   // Distance to maintain when hugging wall
#define DIST_BOX_PICKUP   5    // Distance to grab box

// Color sensor thresholds
#define COLOR_FREQ_MAX    150
#define COLOR_FREQ_BLACK  200
#define COLOR_MARGIN      20

// Servo positions
#define SERVO_CLAMP_OPEN    90
#define SERVO_CLAMP_CLOSED  0
#define SERVO_ARM_DOWN      0
#define SERVO_ARM_CARRY     45

// Timing
#define TIME_TURN_90      500
#define TIME_SERVO_MOVE   300

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                              DATA TYPES                                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

enum Color { 
  COLOR_NONE, 
  COLOR_BLACK,   // Start/end line
  COLOR_WHITE, 
  COLOR_RED,     // Obstacle course path
  COLOR_GREEN, 
  COLOR_BLUE     // Drop zone
};

enum State {
  STATE_FIND_RED,       // Looking for the red line
  STATE_FOLLOW_RED,     // Following the red path
  STATE_APPROACH_BOX,   // Moving toward the box
  STATE_PICKUP,         // Grabbing the box
  STATE_TO_OBSTACLES,   // Heading to obstacle area
  STATE_AVOID_OBS,      // Avoiding an obstacle
  STATE_FIND_BLUE,      // Looking for blue drop zone
  STATE_DROP,           // Dropping the box
  STATE_FIND_BLACK,     // Looking for black line to return
  STATE_RETURN_HOME,    // Following black line back to start
  STATE_COMPLETE        // Done!
};

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                           GLOBAL VARIABLES                                 ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

Servo baseServo, clampServo;
State currentState = STATE_FIND_RED;
bool holding = false;          // Is robot holding a box?
uint32_t stateStartTime = 0;
uint8_t obstacleCount = 0;     // Number of obstacles avoided

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
  
  // Determine color
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

/**
 * Read IR line sensors
 * Returns true if sensor is over black line
 */
void readIR(bool& left, bool& right) {
  left = (digitalRead(PIN_IR_LEFT) == LOW);
  right = (digitalRead(PIN_IR_RIGHT) == LOW);
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

void curveLeft(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, HIGH);
  digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed / 2);                             // LEFT half
  analogWrite(PIN_MOTOR_ENB, (uint8_t)(speed * SPEED_COMPENSATION)); // RIGHT full
}

void curveRight(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, HIGH);
  digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed);                                         // LEFT full
  analogWrite(PIN_MOTOR_ENB, (uint8_t)((speed / 2) * SPEED_COMPENSATION));   // RIGHT half
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                          SERVO FUNCTIONS                                   ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * Pick up a box: lower arm, close claw, raise arm
 */
void pickup() {
  baseServo.write(SERVO_ARM_DOWN);
  delay(TIME_SERVO_MOVE + 200);
  clampServo.write(SERVO_CLAMP_CLOSED);
  delay(TIME_SERVO_MOVE);
  baseServo.write(SERVO_ARM_CARRY);
  delay(TIME_SERVO_MOVE);
  holding = true;
}

/**
 * Drop a box: lower arm, open claw, raise arm
 */
void drop() {
  baseServo.write(SERVO_ARM_DOWN);
  delay(TIME_SERVO_MOVE + 200);
  clampServo.write(SERVO_CLAMP_OPEN);
  delay(TIME_SERVO_MOVE);
  baseServo.write(SERVO_ARM_CARRY);
  delay(TIME_SERVO_MOVE);
  holding = false;
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                        OBSTACLE AVOIDANCE                                  ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * Avoid an obstacle by going around it to the right.
 * 
 * MANEUVER:
 *   1. Turn right 90°
 *   2. Move forward (alongside obstacle)
 *   3. Turn left 90°
 *   4. Move forward while hugging wall
 *   5. Turn left 90° (past obstacle)
 *   6. Move forward (clear of obstacle)
 *   7. Turn right 90° (resume original direction)
 */
void avoidObstacle() {
  Serial.println(F(">>> AVOIDING OBSTACLE <<<"));
  stopMotors();
  delay(100);
  
  // Step 1: Turn right 90°
  turnRight(SPEED_TURN);
  delay(TIME_TURN_90);
  stopMotors();
  delay(100);
  
  // Step 2: Move forward alongside obstacle
  moveForward(SPEED_NORMAL);
  delay(800);
  
  // Step 3: Turn left 90°
  turnLeft(SPEED_TURN);
  delay(TIME_TURN_90);
  stopMotors();
  delay(100);
  
  // Step 4: Wall hug - move forward while maintaining distance
  for (int i = 0; i < 20; i++) {
    float d = readDistance();
    if (d < DIST_WALL_HUG - 3) {
      curveRight(SPEED_NORMAL);  // Too close, veer away
    } else if (d < DIST_WALL_HUG + 5) {
      moveForward(SPEED_NORMAL); // Good distance
    } else {
      break;  // Clear of obstacle
    }
    delay(50);
  }
  stopMotors();
  
  // Step 5: Turn left 90°
  turnLeft(SPEED_TURN);
  delay(TIME_TURN_90);
  stopMotors();
  delay(100);
  
  // Step 6: Clear the obstacle
  moveForward(SPEED_NORMAL);
  delay(800);
  
  // Step 7: Turn right to resume direction
  turnRight(SPEED_TURN);
  delay(TIME_TURN_90);
  stopMotors();
  
  obstacleCount++;
  Serial.print(F("Obstacles avoided: "));
  Serial.println(obstacleCount);
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                         LINE FOLLOWING                                     ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

/**
 * Follow black line using IR sensors
 */
void followBlackLine() {
  bool left, right;
  readIR(left, right);
  
  if (left && right) {
    moveForward(SPEED_NORMAL);
  } else if (left && !right) {
    curveLeft(SPEED_NORMAL);
  } else if (!left && right) {
    curveRight(SPEED_NORMAL);
  } else {
    moveForward(SPEED_SLOW);
  }
}

/**
 * Follow red line using color sensor
 */
void followRedLine() {
  Color c = readColor();
  if (c == COLOR_RED) {
    moveForward(SPEED_NORMAL);
  } else {
    moveForward(SPEED_SLOW);
  }
}

// ╔═══════════════════════════════════════════════════════════════════════════╗
// ║                           STATE MACHINE                                    ║
// ╚═══════════════════════════════════════════════════════════════════════════╝

void transitionTo(State newState) {
  currentState = newState;
  stateStartTime = millis();
  Serial.print(F("STATE: "));
  Serial.println(newState);
}

void processState() {
  Color color = readColor();
  float dist = readDistance();
  
  switch (currentState) {
    
    // ─────────────────────────────────────────────────────────────────────────
    // FIND RED: Look for the red line (turn left from intersection)
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_FIND_RED:
      turnLeft(SPEED_TURN);
      delay(TIME_TURN_90);
      stopMotors();
      
      if (readColor() == COLOR_RED) {
        transitionTo(STATE_FOLLOW_RED);
      } else {
        moveForward(SPEED_SLOW);
        delay(300);
        stopMotors();
      }
      
      // Timeout
      if (millis() - stateStartTime > 3000) {
        transitionTo(STATE_FOLLOW_RED);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // FOLLOW RED: Follow the red line, watch for box or obstacles
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_FOLLOW_RED:
      followRedLine();
      
      // Not holding box? Look for box
      if (!holding && dist < DIST_BOX_PICKUP + 10 && dist > 0) {
        transitionTo(STATE_APPROACH_BOX);
      }
      
      // Holding box? Watch for obstacles
      if (holding && dist < DIST_OBSTACLE && dist > 0) {
        transitionTo(STATE_TO_OBSTACLES);
      }
      
      // After 2 obstacles, look for blue
      if (holding && obstacleCount >= 2 && color == COLOR_BLUE) {
        transitionTo(STATE_FIND_BLUE);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // APPROACH BOX: Move slowly toward the box
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_APPROACH_BOX:
      if (dist <= DIST_BOX_PICKUP) {
        stopMotors();
        transitionTo(STATE_PICKUP);
      } else {
        moveForward(SPEED_SLOW);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // PICKUP: Grab the box
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_PICKUP:
      pickup();
      transitionTo(STATE_TO_OBSTACLES);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // TO OBSTACLES: Continue on path until obstacle detected
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_TO_OBSTACLES:
      followRedLine();
      
      // Obstacle detected?
      if (dist < DIST_OBSTACLE && dist > 0) {
        transitionTo(STATE_AVOID_OBS);
      }
      
      // Done with obstacles? Look for blue
      if (obstacleCount >= 2 && color == COLOR_BLUE) {
        transitionTo(STATE_FIND_BLUE);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // AVOID OBS: Execute obstacle avoidance maneuver
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_AVOID_OBS:
      avoidObstacle();
      
      if (obstacleCount >= 2) {
        transitionTo(STATE_FIND_BLUE);
      } else {
        transitionTo(STATE_TO_OBSTACLES);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // FIND BLUE: Look for the blue drop zone
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_FIND_BLUE:
      if (color == COLOR_BLUE) {
        moveForward(SPEED_SLOW);
        delay(500);
        stopMotors();
        transitionTo(STATE_DROP);
      } else {
        followRedLine();
        // Search pattern if taking too long
        if (millis() - stateStartTime > 5000) {
          turnLeft(SPEED_TURN);
          delay(200);
          stopMotors();
        }
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // DROP: Release the box
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_DROP:
      drop();
      transitionTo(STATE_FIND_BLACK);
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // FIND BLACK: Look for black line to return home
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_FIND_BLACK:
      if (color == COLOR_BLACK) {
        transitionTo(STATE_RETURN_HOME);
      } else if (color == COLOR_RED) {
        followRedLine();
      } else {
        moveForward(SPEED_SLOW);
        delay(200);
      }
      
      // Timeout
      if (millis() - stateStartTime > 5000) {
        transitionTo(STATE_RETURN_HOME);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // RETURN HOME: Follow black line back to start
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_RETURN_HOME:
      followBlackLine();
      
      // Assume we're home after 5 seconds
      if (millis() - stateStartTime > 5000) {
        stopMotors();
        transitionTo(STATE_COMPLETE);
      }
      break;
    
    // ─────────────────────────────────────────────────────────────────────────
    // COMPLETE: Competition finished!
    // ─────────────────────────────────────────────────────────────────────────
    case STATE_COMPLETE:
      stopMotors();
      Serial.println(F("\n╔═══════════════════════════════════╗"));
      Serial.println(F("║     COMPETITION COMPLETE!         ║"));
      Serial.println(F("╚═══════════════════════════════════╝"));
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
  
  // IR sensor pins
  pinMode(PIN_IR_LEFT, INPUT);
  pinMode(PIN_IR_RIGHT, INPUT);
  
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
  baseServo.write(SERVO_ARM_CARRY);
  clampServo.write(SERVO_CLAMP_OPEN);
  
  stopMotors();
  delay(1000);
  
  Serial.println(F("============================="));
  Serial.println(F("  SECTION 3: OBSTACLE COURSE"));
  Serial.println(F("============================="));
  Serial.println(F("Mission: Red line -> Box -> Obstacles -> Blue -> Home"));
  Serial.println();
  
  transitionTo(STATE_FIND_RED);
}

void loop() {
  processState();
  delay(50);
}
