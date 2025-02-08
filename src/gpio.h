#ifndef GPIO_H
#define GPIO_H

#include <pthread.h>
#include <stdatomic.h>

struct gpio_pin_t {
    unsigned int pin;
    unsigned int pwm;
    unsigned int min;
    unsigned int max;
    atomic_uint new_value;
    unsigned int _current_value;
    unsigned int sensitivity;
    pthread_mutex_t mtx;
    atomic_bool should_stop;
    pthread_t handle;
};

int gpio_start(struct gpio_pin_t *pins, size_t num_pins);
void set_gpio_from_scale(struct gpio_pin_t *pin, const float scale);
void gpio_stop(struct gpio_pin_t *pins, size_t num_pins);

#endif // GPIO_H
