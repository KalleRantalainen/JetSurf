#ifndef MOTORCONTROL_H_
#define MOTORCONTROL_H_

#include <stdbool.h>

#include "esc.h"

// Motor has a max and min speed +
// an ESC to control it.
typedef struct {
    int minSpeed;
    int maxSpeed;
    ESC esc;
} Motor;

bool setSpeed(int speed, Motor *motor);
void initMotor(int gpioPin, Motor *motor);

#endif // MOTORCONTROL_H_