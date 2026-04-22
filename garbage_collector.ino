// ============================================================
//  GARBAGE COLLECTOR ROBOT v2 — Arduino + L298N + 2x IR + 2x BO Motor
//  FIXES APPLIED:
//    1. Motor direction reversed (both motors)
//    2. IR logic flipped: stops on BLACK, moves on WHITE
//    3. Motor speed reduced
//    4. Turning logic added based on individual IR sensor readings
// ============================================================

// ──────────────────────────────────────────────
//  USER-CONFIGURABLE VARIABLES
// ──────────────────────────────────────────────
const unsigned long CHECKPOINT_DELAY = 5000;  // ms to pause at each checkpoint (5 seconds)
const int TOTAL_CHECKPOINTS          = 3;     // CP1, CP2, CP3 — 4th detection = final halt
const int MOTOR_SPEED                = 120;   // ✅ FIX 3: Reduced speed (was 180). Range: 0–255
const int TURN_SPEED                 = 100;   // Speed used while correcting direction

// ──────────────────────────────────────────────
//  PIN DEFINITIONS
// ──────────────────────────────────────────────

// L298N — Left Motor
const int LEFT_EN  = 5;   // PWM speed
const int LEFT_IN1 = 6;   // Direction A
const int LEFT_IN2 = 7;   // Direction B

// L298N — Right Motor
const int RIGHT_EN  = 10; // PWM speed
const int RIGHT_IN1 = 8;  // Direction A
const int RIGHT_IN2 = 9;  // Direction B

// IR Sensors
// ✅ FIX 2: Most IR modules output LOW on BLACK, HIGH on WHITE
// We define BLACK_DETECTED as LOW — flip to HIGH if your module is inverted
const int IR_LEFT  = 2;
const int IR_RIGHT = 3;
#define BLACK_DETECTED LOW   // ← Change to HIGH if logic is still inverted on your module
#define WHITE_DETECTED HIGH

// ──────────────────────────────────────────────
//  STATE
// ──────────────────────────────────────────────
int  checkpointCount = 0;
bool robotHalted     = false;

// ──────────────────────────────────────────────
//  MOTOR HELPERS
// ──────────────────────────────────────────────

// ✅ FIX 1: Motor direction reversed — IN1/IN2 and IN3/IN4 swapped
void motorForward() {
  analogWrite(LEFT_EN,  MOTOR_SPEED);
  digitalWrite(LEFT_IN1, LOW);    // ← was HIGH
  digitalWrite(LEFT_IN2, HIGH);   // ← was LOW

  analogWrite(RIGHT_EN,  MOTOR_SPEED);
  digitalWrite(RIGHT_IN1, LOW);   // ← was HIGH
  digitalWrite(RIGHT_IN2, HIGH);  // ← was LOW
}

void motorStop() {
  analogWrite(LEFT_EN,  0);
  analogWrite(RIGHT_EN, 0);
  digitalWrite(LEFT_IN1,  LOW);
  digitalWrite(LEFT_IN2,  LOW);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, LOW);
}

// ✅ FIX 4: Turning helpers — used when only one IR sees black (line correction)
void turnLeft() {
  // Stop left motor, run right motor → robot steers left
  analogWrite(LEFT_EN,  0);
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, LOW);

  analogWrite(RIGHT_EN,  TURN_SPEED);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, HIGH);
}

void turnRight() {
  // Run left motor, stop right motor → robot steers right
  analogWrite(LEFT_EN,  TURN_SPEED);
  digitalWrite(LEFT_IN1, LOW);
  digitalWrite(LEFT_IN2, HIGH);

  analogWrite(RIGHT_EN,  0);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, LOW);
}

// ──────────────────────────────────────────────
//  IR SENSOR READING
// ──────────────────────────────────────────────

// ✅ FIX 2: Returns true when sensor sees BLACK surface
bool leftOnBlack() {
  return (digitalRead(IR_LEFT) == BLACK_DETECTED);
}

bool rightOnBlack() {
  return (digitalRead(IR_RIGHT) == BLACK_DETECTED);
}

bool bothOnBlack() {
  return leftOnBlack() && rightOnBlack();
}

bool bothOnWhite() {
  return !leftOnBlack() && !rightOnBlack();
}

// ──────────────────────────────────────────────
//  DEBOUNCED CHECKPOINT DETECTION
//  Requires 3 consecutive black reads to confirm
// ──────────────────────────────────────────────
bool stableCheckpointDetected() {
  const int CONFIRM_COUNT = 3;
  const int CONFIRM_DELAY = 10; // ms between checks

  int hits = 0;
  for (int i = 0; i < CONFIRM_COUNT; i++) {
    if (bothOnBlack()) hits++;
    delay(CONFIRM_DELAY);
  }
  return (hits == CONFIRM_COUNT);
}

// ──────────────────────────────────────────────
//  SETUP
// ──────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  pinMode(LEFT_EN,   OUTPUT);
  pinMode(LEFT_IN1,  OUTPUT);
  pinMode(LEFT_IN2,  OUTPUT);
  pinMode(RIGHT_EN,  OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  pinMode(IR_LEFT,  INPUT);
  pinMode(IR_RIGHT, INPUT);

  motorStop();
  delay(1000); // power-on settle
  Serial.println("Garbage Collector Robot v2 — Starting...");
}

// ──────────────────────────────────────────────
//  MAIN LOOP
// ──────────────────────────────────────────────
void loop() {

  // Safety: keep halted if final destination was reached
  if (robotHalted) {
    motorStop();
    return;
  }

  // Read both sensors
  bool L = leftOnBlack();
  bool R = rightOnBlack();

  // ────────────────────────────────────────────
  // ✅ FIX 4: IR-based navigation logic
  //
  //  BOTH WHITE  → move forward (on open path)
  //  LEFT BLACK only  → veer right (robot drifted left)
  //  RIGHT BLACK only → veer left  (robot drifted right)
  //  BOTH BLACK  → checkpoint or final destination
  // ────────────────────────────────────────────

  if (!L && !R) {
    // Both on WHITE → normal forward movement
    motorForward();

  } else if (L && !R) {
    // Only LEFT sees black → correct by turning right
    Serial.println("Left on black — turning right");
    turnRight();

  } else if (!L && R) {
    // Only RIGHT sees black → correct by turning left
    Serial.println("Right on black — turning left");
    turnLeft();

  } else if (L && R) {
    // BOTH on BLACK → checkpoint / final destination
    if (stableCheckpointDetected()) {
      motorStop();
      checkpointCount++;

      Serial.print(">>> Black tape confirmed — count: ");
      Serial.println(checkpointCount);

      if (checkpointCount <= TOTAL_CHECKPOINTS) {
        // ── MID-ROUTE CHECKPOINT ──
        Serial.print("Checkpoint ");
        Serial.print(checkpointCount);
        Serial.print(" reached. Waiting ");
        Serial.print(CHECKPOINT_DELAY / 1000);
        Serial.println("s...");

        delay(CHECKPOINT_DELAY);
        Serial.println("Resuming...");
        // Loop continues → next iteration resumes movement

      } else {
        // ── FINAL DESTINATION ──
        Serial.println("=== FINAL DESTINATION REACHED — Halting. ===");
        robotHalted = true;
        motorStop();
      }
    }
  }
}
