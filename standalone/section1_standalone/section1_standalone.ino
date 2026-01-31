/**
 * @file section1_standalone.ino
 * @brief SECTION 1: START SECTION - Standalone version
 * @details All code in one file - works on any computer
 * 
 * MISSION: Black line -> Pick box -> Green path -> Drop at blue
 */

#include <Servo.h>

// ============================================================================
// PIN DEFINITIONS
// ============================================================================
#define PIN_COLOR_S0      2
#define PIN_COLOR_S1      3
#define PIN_COLOR_S2      4
#define PIN_COLOR_S3      5
#define PIN_COLOR_OUT     6

#define PIN_MOTOR_ENA     9
#define PIN_MOTOR_IN1     8
#define PIN_MOTOR_IN2     7
#define PIN_MOTOR_ENB     10
#define PIN_MOTOR_IN3     11
#define PIN_MOTOR_IN4     13

#define PIN_SERVO_BASE    12
#define PIN_SERVO_CLAMP   A4

#define PIN_ULTRA_TRIG    A0
#define PIN_ULTRA_ECHO    A1

#define PIN_IR_LEFT       A2
#define PIN_IR_RIGHT      A3

// ============================================================================
// CONSTANTS
// ============================================================================
#define SPEED_SLOW        100
#define SPEED_NORMAL      150
#define SPEED_TURN        120

#define DIST_BOX_PICKUP   5
#define COLOR_FREQ_MAX    150
#define COLOR_FREQ_BLACK  200
#define COLOR_MARGIN      20

#define SERVO_CLAMP_OPEN    90
#define SERVO_CLAMP_CLOSED  0
#define SERVO_ARM_DOWN      0
#define SERVO_ARM_CARRY     45

#define TIME_TURN_90      500
#define TIME_SERVO_MOVE   300

// ============================================================================
// ENUMS & STATE
// ============================================================================
enum Color { COLOR_NONE, COLOR_BLACK, COLOR_WHITE, COLOR_RED, COLOR_GREEN, COLOR_BLUE };
enum State {
  STATE_FOLLOW_BLACK,
  STATE_APPROACH_BOX,
  STATE_PICKUP,
  STATE_FIND_INTERSECTION,
  STATE_SELECT_GREEN,
  STATE_FOLLOW_GREEN,
  STATE_APPROACH_BLUE,
  STATE_DROP,
  STATE_TO_REUPLOAD,
  STATE_COMPLETE
};

// ============================================================================
// GLOBALS
// ============================================================================
Servo baseServo, clampServo;
State currentState = STATE_FOLLOW_BLACK;
bool holding = false;
uint32_t stateStartTime = 0;

// ============================================================================
// SENSOR FUNCTIONS
// ============================================================================
float readDistance() {
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_ULTRA_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_ULTRA_TRIG, LOW);
  unsigned long d = pulseIn(PIN_ULTRA_ECHO, HIGH, 25000);
  return d == 0 ? 999.0 : (d * 0.034) / 2.0;
}

Color readColor() {
  digitalWrite(PIN_COLOR_S2, LOW); digitalWrite(PIN_COLOR_S3, LOW);
  delay(10);
  uint16_t r = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  digitalWrite(PIN_COLOR_S2, HIGH); digitalWrite(PIN_COLOR_S3, HIGH);
  delay(10);
  uint16_t g = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  digitalWrite(PIN_COLOR_S2, LOW); digitalWrite(PIN_COLOR_S3, HIGH);
  delay(10);
  uint16_t b = pulseIn(PIN_COLOR_OUT, LOW, 40000);
  
  if (r == 0) r = 999; if (g == 0) g = 999; if (b == 0) b = 999;
  
  if (r > COLOR_FREQ_BLACK && g > COLOR_FREQ_BLACK && b > COLOR_FREQ_BLACK) return COLOR_BLACK;
  if (r < g - COLOR_MARGIN && r < b - COLOR_MARGIN && r < COLOR_FREQ_MAX) return COLOR_RED;
  if (g < r - COLOR_MARGIN && g < b - COLOR_MARGIN && g < COLOR_FREQ_MAX) return COLOR_GREEN;
  if (b < r - COLOR_MARGIN && b < g - COLOR_MARGIN && b < COLOR_FREQ_MAX) return COLOR_BLUE;
  return COLOR_NONE;
}

void readIR(bool& left, bool& right) {
  left = (digitalRead(PIN_IR_LEFT) == LOW);
  right = (digitalRead(PIN_IR_RIGHT) == LOW);
}

// ============================================================================
// MOTOR FUNCTIONS
// ============================================================================
void stopMotors() {
  analogWrite(PIN_MOTOR_ENA, 0);
  analogWrite(PIN_MOTOR_ENB, 0);
}

void moveForward(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH); digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, HIGH); digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed); analogWrite(PIN_MOTOR_ENB, speed);
}

void curveLeft(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH); digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, HIGH); digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed); analogWrite(PIN_MOTOR_ENB, speed / 2);
}

void curveRight(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH); digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, HIGH); digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed / 2); analogWrite(PIN_MOTOR_ENB, speed);
}

void turnLeft(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, LOW); digitalWrite(PIN_MOTOR_IN2, HIGH);
  digitalWrite(PIN_MOTOR_IN3, HIGH); digitalWrite(PIN_MOTOR_IN4, LOW);
  analogWrite(PIN_MOTOR_ENA, speed); analogWrite(PIN_MOTOR_ENB, speed);
}

void turnRight(uint8_t speed) {
  digitalWrite(PIN_MOTOR_IN1, HIGH); digitalWrite(PIN_MOTOR_IN2, LOW);
  digitalWrite(PIN_MOTOR_IN3, LOW); digitalWrite(PIN_MOTOR_IN4, HIGH);
  analogWrite(PIN_MOTOR_ENA, speed); analogWrite(PIN_MOTOR_ENB, speed);
}

// ============================================================================
// SERVO FUNCTIONS
// ============================================================================
void pickup() {
  baseServo.write(SERVO_ARM_DOWN); delay(TIME_SERVO_MOVE + 200);
  clampServo.write(SERVO_CLAMP_CLOSED); delay(TIME_SERVO_MOVE);
  baseServo.write(SERVO_ARM_CARRY); delay(TIME_SERVO_MOVE);
  holding = true;
}

void drop() {
  baseServo.write(SERVO_ARM_DOWN); delay(TIME_SERVO_MOVE + 200);
  clampServo.write(SERVO_CLAMP_OPEN); delay(TIME_SERVO_MOVE);
  baseServo.write(SERVO_ARM_CARRY); delay(TIME_SERVO_MOVE);
  holding = false;
}

// ============================================================================
// LINE FOLLOWING
// ============================================================================
void followBlackLine() {
  bool left, right;
  readIR(left, right);
  
  if (left && right) moveForward(SPEED_NORMAL);
  else if (left && !right) curveLeft(SPEED_NORMAL);
  else if (!left && right) curveRight(SPEED_NORMAL);
  else moveForward(SPEED_SLOW);
}

void followGreenLine() {
  Color c = readColor();
  if (c == COLOR_GREEN) moveForward(SPEED_NORMAL);
  else moveForward(SPEED_SLOW);
}

// ============================================================================
// STATE MACHINE
// ============================================================================
void transitionTo(State s) {
  currentState = s;
  stateStartTime = millis();
  Serial.print(F("STATE: ")); Serial.println(s);
}

void processState() {
  float dist = readDistance();
  Color color = readColor();
  
  switch (currentState) {
    case STATE_FOLLOW_BLACK:
      followBlackLine();
      if (dist < DIST_BOX_PICKUP + 10 && dist > 0) transitionTo(STATE_APPROACH_BOX);
      break;
      
    case STATE_APPROACH_BOX:
      if (dist <= DIST_BOX_PICKUP) { stopMotors(); transitionTo(STATE_PICKUP); }
      else moveForward(SPEED_SLOW);
      break;
      
    case STATE_PICKUP:
      pickup();
      transitionTo(STATE_FIND_INTERSECTION);
      break;
      
    case STATE_FIND_INTERSECTION:
      followBlackLine();
      if (color == COLOR_GREEN || color == COLOR_RED) {
        stopMotors();
        transitionTo(STATE_SELECT_GREEN);
      }
      break;
      
    case STATE_SELECT_GREEN:
      if (color == COLOR_GREEN) { transitionTo(STATE_FOLLOW_GREEN); break; }
      turnLeft(SPEED_TURN);
      delay(300);
      stopMotors();
      if (readColor() == COLOR_GREEN) transitionTo(STATE_FOLLOW_GREEN);
      else { turnRight(SPEED_TURN); delay(600); stopMotors(); transitionTo(STATE_FOLLOW_GREEN); }
      break;
      
    case STATE_FOLLOW_GREEN:
      followGreenLine();
      if (color == COLOR_BLUE) transitionTo(STATE_APPROACH_BLUE);
      break;
      
    case STATE_APPROACH_BLUE:
      moveForward(SPEED_SLOW);
      delay(500);
      stopMotors();
      transitionTo(STATE_DROP);
      break;
      
    case STATE_DROP:
      drop();
      transitionTo(STATE_TO_REUPLOAD);
      break;
      
    case STATE_TO_REUPLOAD:
      followGreenLine();
      if (millis() - stateStartTime > 3000) {
        stopMotors();
        transitionTo(STATE_COMPLETE);
      }
      break;
      
    case STATE_COMPLETE:
      stopMotors();
      Serial.println(F("\n=== SECTION 1 COMPLETE ==="));
      Serial.println(F("Upload section2_standalone.ino"));
      while (true) delay(1000);
      break;
  }
}

// ============================================================================
// SETUP & LOOP
// ============================================================================
void setup() {
  Serial.begin(9600);
  
  pinMode(PIN_COLOR_S0, OUTPUT); pinMode(PIN_COLOR_S1, OUTPUT);
  pinMode(PIN_COLOR_S2, OUTPUT); pinMode(PIN_COLOR_S3, OUTPUT);
  pinMode(PIN_COLOR_OUT, INPUT);
  digitalWrite(PIN_COLOR_S0, HIGH); digitalWrite(PIN_COLOR_S1, LOW);
  
  pinMode(PIN_ULTRA_TRIG, OUTPUT); pinMode(PIN_ULTRA_ECHO, INPUT);
  pinMode(PIN_IR_LEFT, INPUT); pinMode(PIN_IR_RIGHT, INPUT);
  
  pinMode(PIN_MOTOR_ENA, OUTPUT); pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT); pinMode(PIN_MOTOR_ENB, OUTPUT);
  pinMode(PIN_MOTOR_IN3, OUTPUT); pinMode(PIN_MOTOR_IN4, OUTPUT);
  
  baseServo.attach(PIN_SERVO_BASE);
  clampServo.attach(PIN_SERVO_CLAMP);
  baseServo.write(SERVO_ARM_DOWN);
  clampServo.write(SERVO_CLAMP_OPEN);
  
  stopMotors();
  delay(1000);
  
  Serial.println(F("=== SECTION 1: START ==="));
  transitionTo(STATE_FOLLOW_BLACK);
}

void loop() {
  processState();
  delay(50);
}
