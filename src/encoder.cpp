#include "config.h"
#include <Arduino.h>

// --- Rotary Encoder State Variables ---
// Using the stable counter-based ISR logic
volatile int encoderCounter = 0;
volatile int lastKnownPosition = HIGH;

// --- Button State Variables ---
int buttonState;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
bool buttonClicked = false;

// --- Interrupt Service Routine (ISR) ---
// This is the stable ISR you provided. It increments/decrements a persistent counter.
void IRAM_ATTR handleEncoderRotation() {
  int newClockState = digitalRead(ENCODER_CLK_PIN);
  if (newClockState != lastKnownPosition) {
    // A transition has occurred
    if (newClockState == LOW) {
      // We act on the falling edge
      if (digitalRead(ENCODER_DT_PIN) == LOW) {
        encoderCounter--; // Direction 1
      } else {
        encoderCounter++; // Direction 2
      }
    }
    lastKnownPosition = newClockState;
  }
}

void setup_encoder() {
  pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP);
  
  // Initialize last known position for the ISR
  lastKnownPosition = digitalRead(ENCODER_CLK_PIN);
  // Attach the interrupt to trigger on any state change
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), handleEncoderRotation, CHANGE);

  // Initialize button state
  lastButtonState = digitalRead(ENCODER_SW_PIN);
}

void loop_encoder() {
    // Read the current state of the switch
    int reading = digitalRead(ENCODER_SW_PIN);

    // If the switch changed, due to noise or pressing
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > 50) {
        // Whatever the reading is at, it's been there for a while
        // so we assume that it's the stable state
        if (reading != buttonState) {
            buttonState = reading;

            // If the button state has changed to LOW (pressed)
            if (buttonState == LOW) {
                buttonClicked = true;
            }
        }
    }
    lastButtonState = reading;
}

int get_encoder_value() {
  int value;
  noInterrupts(); // Disable interrupts to safely read multi-byte variable
  value = encoderCounter;
  interrupts(); // Re-enable interrupts
  return value;
}

bool button_was_clicked() {
    if (buttonClicked) {
        buttonClicked = false;
        return true;
    }
    return false;
}

