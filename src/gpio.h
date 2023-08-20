#ifndef SERVO_H
#define SERVO_H

#include "steering.h"

#define MIN_WIDTH 500
#define MID_WIDTH 1500
#define MAX_WIDTH 2500

#define SERVO_RANGE (MAX_WIDTH - MIN_WIDTH)

#define GPIO_STEER_X 17

int gpio_start(steering_t* movement);
void set_servo_rotation(int pin, float scale);
void gpio_stop(void);

#endif  // SERVO_H