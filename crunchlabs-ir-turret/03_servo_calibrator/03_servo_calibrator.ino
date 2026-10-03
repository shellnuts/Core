// Servo Calibrator
// ----------------
// Find the right tuning numbers for YOUR turret, then copy them into
// 02_turret_fun_pack. Open the Serial Monitor at 9600 baud and set the line
// ending to "Newline". Then type commands and press Enter:
//
//   y / p / r   choose a servo: yaw (left/right), pitch (up/down), roll (barrel)
//   a number    send that value to the chosen servo, e.g. 90 or 88
//   t158        TIMED TEST: spin the chosen yaw/roll servo at full speed for
//               158 ms, then stop. Use it to tune ROLL_ONE_DART_MS.
//   ?           show this help again
//
// Finding the stop value (YAW_STOP / ROLL_STOP):
//   type y, then try 90, 89, 91, 88, 92... until the turret stays perfectly
//   still. The LAST number you typed is used as the stop value for t tests.
//
// SAFETY: take the darts out before testing the roll servo!

#include <Arduino.h>
#include <Servo.h>

const uint8_t YAW_PIN   = 10;
const uint8_t PITCH_PIN = 11;
const uint8_t ROLL_PIN  = 12;
const int FULL_SPEED    = 90;   // how far from the stop value a timed test spins

Servo yawServo;
Servo pitchServo;
Servo rollServo;

char chosen = 'y';      // which servo commands go to: 'y', 'p' or 'r'
int yawStop  = 90;      // last value typed for each servo
int pitchPos = 100;
int rollStop = 90;

const __FlashStringHelper* chosenName() {
  if (chosen == 'y') return F("YAW (pin 10, continuous)");
  if (chosen == 'p') return F("PITCH (pin 11, angle)");
  return F("ROLL (pin 12, continuous)");
}

void printHelp() {
  Serial.println(F("=== Servo Calibrator ==="));
  Serial.println(F("y/p/r = choose servo, number = send value,"));
  Serial.println(F("t<ms> = timed spin test (yaw/roll), ? = help"));
  Serial.print(F("Chosen servo: "));
  Serial.println(chosenName());
}

void sendValue(int value) {
  if (chosen == 'y') {
    yawStop = constrain(value, 0, 180);
    yawServo.write(yawStop);
  } else if (chosen == 'p') {
    pitchPos = constrain(value, 10, 175);   // stay inside the safe pitch range
    pitchServo.write(pitchPos);
    if (pitchPos != value) Serial.println(F("(limited to 10-175 to protect the servo)"));
    value = pitchPos;
  } else {
    rollStop = constrain(value, 0, 180);
    rollServo.write(rollStop);
  }
  Serial.print(F("Sent "));
  Serial.print(value);
  Serial.print(F(" to "));
  Serial.println(chosenName());
}

void timedTest(int ms) {
  if (chosen == 'p') {
    Serial.println(F("Timed tests are for yaw and roll. Type a number for pitch."));
    return;
  }
  Servo& servo = (chosen == 'y') ? yawServo : rollServo;
  int stopValue = (chosen == 'y') ? yawStop : rollStop;

  Serial.print(F("Spinning for "));
  Serial.print(ms);
  Serial.print(F(" ms, then stopping at "));
  Serial.println(stopValue);
  servo.write(constrain(stopValue + FULL_SPEED, 0, 180));
  delay(ms);
  servo.write(stopValue);
}

void setup() {
  Serial.begin(9600);
  yawServo.attach(YAW_PIN);
  pitchServo.attach(PITCH_PIN);
  rollServo.attach(ROLL_PIN);
  yawServo.write(yawStop);
  pitchServo.write(pitchPos);
  rollServo.write(rollStop);
  printHelp();
}

void loop() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;
  char first = tolower(line.charAt(0));

  if (first == 'y' || first == 'p' || first == 'r') {
    chosen = first;
    Serial.print(F("Chosen servo: "));
    Serial.println(chosenName());
  } else if (first == 't') {
    int ms = line.substring(1).toInt();
    if (ms > 0 && ms <= 5000) timedTest(ms);
    else Serial.println(F("Type t and a time from 1 to 5000, like t158"));
  } else if (first == '?') {
    printHelp();
  } else if (isDigit(first)) {
    sendValue(line.toInt());
  } else {
    Serial.println(F("Unknown command. Type ? for help."));
  }
}
