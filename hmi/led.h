#ifndef LED_H_
#define LED_H_

#include <stdbool.h>

typedef struct {
    int gpioPin;       // To which gpio led is connected to
    bool initialized;  // Is the led initialized
    bool blinking;     // Is the led blinking
    bool ledOn;        // Is the led light
    int onCycles;      // How many cycles to stay off if blinking
    int offCycles;     // -||- on if blinking
    int currentCycles; // For how many cycles the current action has lasted
} led;

// Initialize a led
void initLed(led* led, int gpioPin);
// Start blinking a led with cycleCount
void startBlink(led* led, int cycleCount);
// Stop led blinking, if the led is currently blinking
void stopBlink(led* led);
// Blinks the led according to the set blink variables of the led
void blinker(led* led);


#endif // LED_H_