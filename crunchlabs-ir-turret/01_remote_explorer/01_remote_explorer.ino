// Remote Explorer
// ---------------
// Upload this, open the Serial Monitor (9600 baud), and press buttons on the
// remote. It prints the code for each button so you know what number to use
// in your own projects. Works with any NEC-style IR remote (TV remotes too!).

#include <Arduino.h>
#define DECODE_NEC          // only listen for NEC remotes (saves memory)
#include <IRremote.hpp>

const uint8_t IR_PIN = 9;

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
  Serial.println(F("Remote Explorer ready. Press a button!"));
}

void loop() {
  if (!IrReceiver.decode()) return;

  if (IrReceiver.decodedIRData.protocol != UNKNOWN) {
    bool isRepeat = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;
    Serial.print(F("Button code: 0x"));
    Serial.print(IrReceiver.decodedIRData.command, HEX);
    Serial.println(isRepeat ? F("  (held down)") : F(""));
  }
  IrReceiver.resume();  // get ready for the next button press
}
