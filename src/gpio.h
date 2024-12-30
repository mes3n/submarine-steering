#ifndef GPIO_H
#define GPIO_H

// These values as the micro second intervals for which the
// servo will react. Toggle on/off at these intervals for the
// servo turn turn to its min, mid, and max position.
#define SERVO_MIN 500
#define SERVO_MID 1500
#define SERVO_MAX 2500

#include <stddef.h>

int gpio_start(const unsigned int *offsets, size_t num_offsets);
void set_servo_rotation(int pin, float scale);
void gpio_stop(void);

#endif // GPIO_H
