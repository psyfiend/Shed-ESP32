#ifndef ENCODER_H
#define ENCODER_H

// Public functions available to the rest of the application
void setup_encoder();
void loop_encoder(); // This should be called in the main loop()

// Functions to check the encoder's state
int get_encoder_value(); // This function now returns the persistent counter value
bool button_was_clicked(); // Returns true on a single click, resets after read

#endif // ENCODER_H
