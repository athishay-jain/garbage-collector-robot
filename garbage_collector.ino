// ============================================================
//  GARBAGE COLLECTOR ROBOT — Arduino + L298N + 2x IR + 2x BO Motor
//  Path: CP1 → CP2 → CP3 → Final Destination
// ============================================================

// ──────────────────────────────────────────────
//  USER-CONFIGURABLE VARIABLES
// ──────────────────────────────────────────────
const unsigned long CHECKPOINT_DELAY = 5000;  // ms to pause at each checkpoint
const int TOTAL_CHECKPOINTS = 3;              // CP1, CP2, CP3 (final = 4th detection)
const int MOTOR_SPEED = 180;                  // 0–255, tune for your motors

// ──────────────────────────────────────────────
//  PIN DEFINITIONS
// ──────────────────────────────────────────────

// L298N — Left Motor
const int LEFT_EN  = 5;   // PWM speed pin
const int LEFT_IN1 = 6;   // Direction A
const int LEFT_IN2 = 7;   // Direction B

// L298N — Right Motor
const int RIGHT_EN  = 10; // PWM speed pin
const int RIGHT_IN1 = 8;  // Direction A
const int RIGHT_IN2 = 9;  // Direction B

// IR Sensors (LOW = black detected for most modules; adjust if yours differ)
const int IR_LEFT  = 2;
const int IR_RIGHT = 3;

// ──────────────────────────────────────────────
//  STATE
// ──────────────────────────────────────────────
int  checkpointCount  = 0;
bool robotHalted      = false;

// ──────────────────────────────────────────────
//  HELPERS
// ──────────────────────────────────────────────

void motorForward() {
  // Left motor forward
  analogWrite(LEFT_EN, MOTOR_SPEED);
  digitalWrite(LEFT_IN1, HIGH);
  digitalWrite(LEFT_IN2, LOW);

  // Right motor forward
  analogWrite(RIGHT_EN, MOTOR_SPEED);
  digitalWrite(RIGHT_IN1, HIGH);
  digitalWrite(RIGHT_IN2, LOW);
}

void motorStop() {
  analogWrite(LEFT_EN,  0);
  analogWrite(RIGHT_EN, 0);
  digitalWrite(LEFT_IN1,  LOW);
  digitalWrite(LEFT_IN2,  LOW);
  digitalWrite(RIGHT_IN1, LOW);
  digitalWrite(RIGHT_IN2, LOW);
}

bool bothSensorsOnBlack() {
  // IR modules typically pull LOW on black surface — change == HIGH if yours invert
  bool leftBlack  = (digitalRead(IR_LEFT)  == LOW);
  bool rightBlack = (digitalRead(IR_RIGHT) == LOW);
  return (leftBlack && rightBlack);
}

// ──────────────────────────────────────────────
//  SETUP
// ──────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  // Motor pins
  pinMode(LEFT_EN,   OUTPUT);
  pinMode(LEFT_IN1,  OUTPUT);
  pinMode(LEFT_IN2,  OUTPUT);
  pinMode(RIGHT_EN,  OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  // IR sensor pins
  pinMode(IR_LEFT,  INPUT);
  pinMode(IR_RIGHT, INPUT);

  motorStop();
  delay(1000); // short power-on settle time
  Serial.println("Robot starting...");
}

// ──────────────────────────────────────────────
//  MAIN LOOP
// ──────────────────────────────────────────────
void loop() {
  if (robotHalted) {
    motorStop();   // safety — keep motors off
    return;
  }

  motorForward();

  // Small debounce: both sensors must read black for 3 consecutive checks
  // to avoid false triggers from noise or partial crosses
  if (stableBlackDetection()) {
    motorStop();
    checkpointCount++;
    Serial.print("Black tape hit — count: ");
    Serial.println(checkpointCount);

    if (checkpointCount <= TOTAL_CHECKPOINTS) {
      // ── CHECKPOINT ──
      Serial.print("Checkpoint ");
      Serial.print(checkpointCount);
      Serial.println(" reached. Waiting...");
      delay(CHECKPOINT_DELAY);
      Serial.println("Resuming blindly.");
      // Loop continues → motorForward() on next iteration

    } else {
      // ── FINAL DESTINATION ──
      Serial.println("Final destination reached. Halting.");
      robotHalted = true;
      motorStop();
    }
  }
}

// ──────────────────────────────────────────────
//  DEBOUNCED DETECTION
//  Returns true only if both sensors read black
//  on 3 consecutive checks 10 ms apart.
//  Prevents single-sample noise from triggering.
// ──────────────────────────────────────────────
bool stableBlackDetection() {
  const int CONFIRM_COUNT = 3;
  const int CONFIRM_DELAY = 10; // ms between checks

  int hits = 0;
  for (int i = 0; i < CONFIRM_COUNT; i++) {
    if (bothSensorsOnBlack()) hits++;
    delay(CONFIRM_DELAY);
  }
  return (hits == CONFIRM_COUNT);
}
