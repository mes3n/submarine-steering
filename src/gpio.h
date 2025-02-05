#ifndef GPIO_H
#define GPIO_H

// These values as the micro second intervals for which the
// servo will react. Toggle on/off at these intervals for the
// servo turn turn to its min, mid, and max position.
#define SERVO_MIN 500
#define SERVO_MID 1500
#define SERVO_MAX 2500

#define ESC_PWM 20000
#define ESC_MIN 1000
#define ESC_MAX 2000

#include <stddef.h>

typedef struct {
    int pwm;
    int pin;
} gpio_pin_t;

int gpio_start(const unsigned int *offsets, size_t num_offsets);
void set_servo_rotation(int pin, float scale);
void calibrate_esc(int pin);
void set_motor_speed(int pin, float scale);
void gpio_stop(void);

#endif // GPIO_H
