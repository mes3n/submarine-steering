#ifndef GPIO_H
#define GPIO_H

#include <pthread.h>
#include <stdatomic.h>

/**
 * Struct for defining a gpio pin configuration
 *
 * Should not be modified after `gpio_start` has been called on it except
 * through the `new_value` field. Fields starting with an underscore should be
 * assumed to be private and only used for internal functions
 */
struct gpio_pin_t {
    unsigned int pin;
    /// Setting pwm to 0 means the pins output will not be set in a loop
    unsigned int pwm;
    unsigned int min;
    unsigned int max;
    unsigned int sensitivity;
    /// Request setting the gpio to a new value by updating this value
    atomic_uint new_value;
    atomic_uint _current_value;
    atomic_bool _should_stop;
    pthread_t _handle;
};

/**
 * Initialize system gpio interface and gpio pin objects
 *
 * Also starts a thread for each pin which is used to control its gpio outputs
 * as per the given configurations. These configurations should not be changed
 * until gpio_stop has been called, except through the `new_value` fields
 *
 * @param pins An array of pin object to be initialized
 * @param num_pins Number of pins in pins array
 * @return An integer 0 for success otherwise negative
 */
int gpio_start(const char *gpio_chip, struct gpio_pin_t *pins, size_t num_pins);

/**
 * Update gpio new_value from a scale where -1.0 => min and +1.0 => max
 *
 * @param pin A pointer to the pin which will be modified
 * @param scale A floating scale used to set the new pin value
 */
void set_gpio_from_scale(struct gpio_pin_t *pin, const float scale)
    __attribute__((nonnull));

/**
 * Stop system gpio interface and gpio pin object threads
 *
 * @param pins An array of pin object to be stopped
 * @param num_pins Number of pins in pins array
 */
void gpio_stop(struct gpio_pin_t *pins, size_t num_pins);

#endif // GPIO_H
