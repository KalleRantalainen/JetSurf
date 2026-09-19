#ifndef LEDCONTROL_H_
#define LEDCONTROL_H_

#include <stdbool.h>


// Initialize all HMI leds
void initializeLeds(void);
// Do one cyle of computing for leds
void cycleLeds(void);


#endif // LEDCONTROL_H_