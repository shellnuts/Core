// Turret Fun Pack
// ---------------
// Drive the CrunchLabs IR Turret with the remote, plus six mini projects:
//
//   Arrows = aim         OK = fire one dart     * = fire all darts
//   #      = go home / stop a mode              0 = "I reloaded" (refill count)
//   1 = Dance Party      2 = Patrol (sentry)    3 = Roulette
//   4 = Simon Says       5 = Quick Draw         6 = Rocket Countdown
//   7, 8, 9 = YOUR projects (see myProject7() at the bottom)
//
// Open the Serial Monitor at 9600 baud to see messages and scores.
// SAFETY: aim at targets, never at faces, eyes, or pets.

#include <Arduino.h>
#include <Servo.h>
#define DECODE_NEC          // the turret remote uses the NEC protocol
#include <IRremote.hpp>

// ---------- Pins ----------
const uint8_t IR_PIN    = 9;
const uint8_t YAW_PIN   = 10;  // left/right (continuous rotation servo)
const uint8_t PITCH_PIN = 11;  // up/down (normal servo)
const uint8_t ROLL_PIN  = 12;  // spins the barrel to fire (continuous servo)

// ---------- Remote button codes (find others with 01_remote_explorer) ----------
const uint8_t BTN_LEFT  = 0x08;
const uint8_t BTN_RIGHT = 0x5A;
const uint8_t BTN_UP    = 0x18;
const uint8_t BTN_DOWN  = 0x52;
const uint8_t BTN_OK    = 0x1C;
const uint8_t BTN_STAR  = 0x16;
const uint8_t BTN_HASH  = 0x0D;
const uint8_t BTN_0     = 0x19;
const uint8_t BTN_1     = 0x45;
const uint8_t BTN_2     = 0x46;
const uint8_t BTN_3     = 0x47;
const uint8_t BTN_4     = 0x44;
const uint8_t BTN_5     = 0x40;
const uint8_t BTN_6     = 0x43;
const uint8_t BTN_7     = 0x07;
const uint8_t BTN_8     = 0x15;
const uint8_t BTN_9     = 0x09;
const int NO_BUTTON     = -1;

// ---------- Tuning knobs (change these to calibrate YOUR turret) ----------
const int YAW_STOP     = 90;   // value that makes the yaw servo stand still
const int YAW_SPEED    = 90;   // how fast to turn (0-90)
const int YAW_STEP_MS  = 150;  // how long one arrow tap turns
const int PATROL_SPEED = 25;   // slower speed for patrol mode

const int PITCH_MIN    = 10;   // lowest safe pitch angle
const int PITCH_MAX    = 175;  // highest safe pitch angle
const int PITCH_HOME   = 100;  // "looking straight ahead"
const int PITCH_STEP   = 8;    // degrees per arrow tap
const int PITCH_UP     = 1;    // if UP moves the barrel DOWN, change to -1
const int PITCH_SMOOTH_MS = 4; // smaller = faster tilting

const int ROLL_STOP    = 90;   // value that makes the roll servo stand still
const int ROLL_SPEED   = 90;
const int ROLL_ONE_DART_MS = 158;  // time to spin the barrel by one dart
const int MAX_DARTS    = 6;

const bool PENALTY_DART = true;  // in games, does the turret fire when you lose?

// ---------- State ----------
Servo yawServo;
Servo pitchServo;
Servo rollServo;
int pitchAngle = PITCH_HOME;
int dartsLeft = MAX_DARTS;
bool lastWasRepeat = false;   // was the last button a "held down" repeat?

// =====================================================================
//  BUILDING BLOCKS  - use these to write your own projects!
// =====================================================================

// Returns the button code that was just pressed, or NO_BUTTON.
// Never waits. Sets lastWasRepeat if the button is being held down.
int readButton() {
  if (!IrReceiver.decode()) return NO_BUTTON;
  bool known = IrReceiver.decodedIRData.protocol != UNKNOWN;
  int code = IrReceiver.decodedIRData.command;
  lastWasRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
  IrReceiver.resume();
  return known ? code : NO_BUTTON;
}

// Waits up to timeoutMs for a NEW button press (ignores held-down repeats).
int waitForButton(unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    int b = readButton();
    if (b != NO_BUTTON && !lastWasRepeat) return b;
  }
  return NO_BUTTON;
}

// Throws away button presses that piled up while the turret was busy.
void clearButtons() {
  delay(120);
  while (readButton() != NO_BUTTON) {}
}

// ---- Turning (yaw). Yaw is a continuous servo, so we turn for some TIME. ----
void startTurning(bool left, int speed) {
  yawServo.write(left ? YAW_STOP + speed : YAW_STOP - speed);
}

void stopTurning() {
  yawServo.write(YAW_STOP);
}

void turnLeft(int ms) {
  startTurning(true, YAW_SPEED);
  delay(ms);
  stopTurning();
}

void turnRight(int ms) {
  startTurning(false, YAW_SPEED);
  delay(ms);
  stopTurning();
}

// ---- Tilting (pitch). Pitch is a normal servo, so we move to an ANGLE. ----
void tiltTo(int target) {
  target = constrain(target, PITCH_MIN, PITCH_MAX);
  while (pitchAngle != target) {
    pitchAngle += (target > pitchAngle) ? 1 : -1;
    pitchServo.write(pitchAngle);
    delay(PITCH_SMOOTH_MS);
  }
}

void tiltUp(int degrees)   { tiltTo(pitchAngle + PITCH_UP * degrees); }
void tiltDown(int degrees) { tiltTo(pitchAngle - PITCH_UP * degrees); }

// ---- Firing (roll) ----
void shakeNo();  // defined below

void fire() {
  if (dartsLeft == 0) {
    Serial.println(F("Out of darts! Reload, then press 0."));
    shakeNo();
    return;
  }
  rollServo.write(ROLL_STOP + ROLL_SPEED);
  delay(ROLL_ONE_DART_MS);
  rollServo.write(ROLL_STOP);
  dartsLeft--;
  Serial.print(F("Pew! Darts left: "));
  Serial.println(dartsLeft);
  delay(50);
}

void fireAll() {
  while (dartsLeft > 0) {
    fire();
    delay(150);
  }
}

// ---- Expressions ----
void nodYes() {
  int start = pitchAngle;
  for (int i = 0; i < 2; i++) {
    tiltDown(15);
    tiltTo(start);
  }
}

void shakeNo() {
  for (int i = 0; i < 2; i++) {
    turnLeft(120);
    turnRight(240);
    turnLeft(120);
  }
}

void goHome() {
  stopTurning();
  tiltTo(PITCH_HOME);
}

// =====================================================================
//  PROJECT 1: Dance Party
// =====================================================================
void danceParty() {
  Serial.println(F("Dance party!"));
  for (int i = 0; i < 2; i++) {
    turnLeft(300);
    turnRight(300);
    nodYes();
    turnRight(300);
    turnLeft(300);
    shakeNo();
  }
  turnLeft(1200);                 // spin move!
  tiltTo(PITCH_MAX - 20);
  tiltTo(PITCH_MIN + 20);
  goHome();
}

// =====================================================================
//  PROJECT 2: Patrol - sweeps back and forth like a sentry.
//  OK = fire where it's looking, # = stop patrolling.
// =====================================================================
void patrolMode() {
  Serial.println(F("Patrol mode! OK = fire, # = stop"));
  const unsigned long LEG_MS = 1500;   // how long each sweep lasts
  bool goingLeft = true;
  unsigned long legStart = millis();
  startTurning(goingLeft, PATROL_SPEED);

  while (true) {
    int b = readButton();
    if (b == BTN_HASH) break;
    if (b == BTN_OK && !lastWasRepeat) {
      unsigned long sweptSoFar = millis() - legStart;
      stopTurning();
      fire();
      legStart = millis() - sweptSoFar;   // pick up where we left off
      startTurning(goingLeft, PATROL_SPEED);
    }
    if (millis() - legStart >= LEG_MS) {  // end of a sweep: look around, turn back
      stopTurning();
      nodYes();
      goingLeft = !goingLeft;
      legStart = millis();
      startTurning(goingLeft, PATROL_SPEED);
    }
  }
  stopTurning();
  Serial.println(F("Patrol over."));
}

// =====================================================================
//  PROJECT 3: Roulette - spins to a random spot and fires.
//  Set up targets in a circle and see who gets tagged!
// =====================================================================
void roulette() {
  Serial.println(F("Roulette! Where will it stop?"));
  int spinMs = random(800, 2500);
  if (random(2) == 0) turnLeft(spinMs);
  else turnRight(spinMs);
  tiltTo(PITCH_HOME + random(-25, 26));

  for (int i = 3; i > 0; i--) {   // dramatic countdown
    Serial.println(i);
    tiltDown(6);
    tiltUp(6);
    delay(500);
  }
  fire();
}

// =====================================================================
//  PROJECT 4: Simon Says - the turret shows moves, you copy them with
//  the arrow keys. Each round adds one more move. # quits.
// =====================================================================
const uint8_t SIMON_MAX = 20;
uint8_t simonMoves[SIMON_MAX];

void showMove(int move) {
  if (move == BTN_UP)    { tiltUp(25);    tiltDown(25); }
  if (move == BTN_DOWN)  { tiltDown(25);  tiltUp(25); }
  if (move == BTN_LEFT)  { turnLeft(250); turnRight(250); }
  if (move == BTN_RIGHT) { turnRight(250); turnLeft(250); }
}

void simonSays() {
  const uint8_t choices[4] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT};
  Serial.println(F("Simon Says! Watch, then copy with the arrows."));
  goHome();

  for (uint8_t length = 1; length <= SIMON_MAX; length++) {
    simonMoves[length - 1] = choices[random(4)];
    Serial.print(F("Round "));
    Serial.println(length);
    delay(800);

    for (uint8_t i = 0; i < length; i++) {   // turret shows the pattern
      showMove(simonMoves[i]);
      delay(300);
    }
    clearButtons();

    for (uint8_t i = 0; i < length; i++) {   // player copies it
      int b = waitForButton(5000);
      if (b == BTN_HASH) return;
      if (b != simonMoves[i]) {
        Serial.print(F("Wrong! You reached round "));
        Serial.println(length);
        shakeNo();
        if (PENALTY_DART) fire();
        return;
      }
      showMove(b);   // echo your move back
      clearButtons();
    }
    nodYes();        // round complete!
  }
  Serial.println(F("You beat Simon! Champion!"));
  danceParty();
}

// =====================================================================
//  PROJECT 5: Quick Draw - wait for the turret to DIP, then press OK
//  as fast as you can. Press too early and it's a false start!
// =====================================================================
void quickDraw() {
  Serial.println(F("Quick Draw! Press OK the moment the barrel dips."));
  goHome();
  clearButtons();
  Serial.println(F("Ready..."));
  nodYes();
  Serial.println(F("Set..."));

  unsigned long waitMs = random(1500, 5000);
  unsigned long start = millis();
  while (millis() - start < waitMs) {
    if (readButton() == BTN_OK) {
      Serial.println(F("False start!"));
      shakeNo();
      return;
    }
  }

  pitchServo.write(pitchAngle - PITCH_UP * 30);   // DRAW! (instant dip)
  unsigned long drawTime = millis();
  int b = NO_BUTTON;
  while (b != BTN_OK && millis() - drawTime < 3000) {
    b = waitForButton(3000);
  }
  // Note: the remote takes about 70 ms just to send a button signal.
  unsigned long reaction = millis() - drawTime;
  pitchServo.write(pitchAngle);

  if (b != BTN_OK) {
    Serial.println(F("Too slow - the turret wins!"));
    if (PENALTY_DART) fire();
    return;
  }
  Serial.print(F("Reaction time: "));
  Serial.print(reaction);
  Serial.println(F(" ms"));
  if (reaction < 400) {
    Serial.println(F("You win, fastest hand in the west!"));
    nodYes();
  } else {
    Serial.println(F("The turret was faster!"));
    if (PENALTY_DART) fire();
  }
}

// =====================================================================
//  PROJECT 6: Rocket Countdown - aims up slowly, counts down, fires all.
// =====================================================================
void rocketCountdown() {
  Serial.println(F("Rocket launch sequence started! (# to abort)"));
  goHome();
  int startAngle = pitchAngle;
  int topAngle = (PITCH_UP > 0) ? PITCH_MAX - 30 : PITCH_MIN + 30;

  for (int count = 10; count > 0; count--) {
    Serial.println(count);
    tiltTo(map(count, 10, 1, startAngle, topAngle));
    if (waitForButton(700) == BTN_HASH) {
      Serial.println(F("Launch aborted."));
      goHome();
      return;
    }
  }
  Serial.println(F("LIFTOFF!"));
  fireAll();
  goHome();
}

// =====================================================================
//  YOUR PROJECTS: buttons 7, 8, 9. Try writing your own here!
//  Use the building blocks: turnLeft(ms), turnRight(ms), tiltUp(deg),
//  tiltDown(deg), tiltTo(angle), fire(), nodYes(), shakeNo(),
//  waitForButton(ms), random(min, max), delay(ms), Serial.println(...)
// =====================================================================
void myProject7() {
  Serial.println(F("Project 7: edit me!"));
  // Example: look left, look right, then fire.
  turnLeft(400);
  turnRight(800);
  turnLeft(400);
  fire();
}

void myProject8() {
  Serial.println(F("Project 8: edit me!"));
  nodYes();
}

void myProject9() {
  Serial.println(F("Project 9: edit me!"));
  shakeNo();
}

// =====================================================================
//  SETUP + MAIN LOOP
// =====================================================================
void printHelp() {
  Serial.println(F("=== Turret Fun Pack ==="));
  Serial.println(F("Arrows aim, OK fire, * fire all, # home, 0 reloaded"));
  Serial.println(F("1 Dance  2 Patrol  3 Roulette  4 Simon  5 Quick Draw"));
  Serial.println(F("6 Rocket  7/8/9 your projects"));
}

void setup() {
  Serial.begin(9600);
  yawServo.attach(YAW_PIN);
  pitchServo.attach(PITCH_PIN);
  rollServo.attach(ROLL_PIN);
  yawServo.write(YAW_STOP);
  rollServo.write(ROLL_STOP);
  pitchServo.write(pitchAngle);

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
  randomSeed(analogRead(A0));   // makes random() different every time
  printHelp();
  nodYes();                     // "I'm awake!"
}

void loop() {
  int b = readButton();
  if (b == NO_BUTTON) return;

  bool isArrow = (b == BTN_LEFT || b == BTN_RIGHT || b == BTN_UP || b == BTN_DOWN);
  if (lastWasRepeat && !isArrow) return;   // only arrows repeat when held

  switch (b) {
    case BTN_LEFT:  turnLeft(YAW_STEP_MS);  break;
    case BTN_RIGHT: turnRight(YAW_STEP_MS); break;
    case BTN_UP:    tiltUp(PITCH_STEP);     break;
    case BTN_DOWN:  tiltDown(PITCH_STEP);   break;
    case BTN_OK:    fire();                 break;
    case BTN_STAR:  fireAll();              break;
    case BTN_HASH:  goHome();               break;
    case BTN_0:
      dartsLeft = MAX_DARTS;
      Serial.println(F("Reloaded!"));
      nodYes();
      break;
    case BTN_1: danceParty();      break;
    case BTN_2: patrolMode();      break;
    case BTN_3: roulette();        break;
    case BTN_4: simonSays();       break;
    case BTN_5: quickDraw();       break;
    case BTN_6: rocketCountdown(); break;
    case BTN_7: myProject7();      break;
    case BTN_8: myProject8();      break;
    case BTN_9: myProject9();      break;
  }
  if (!isArrow) clearButtons();   // ignore presses made while busy
}
