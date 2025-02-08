#include "gpio.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef BUILD_PIGPIO
#include <pigpio.h>
#elif defined(BUILD_LIBGPIOD)
#include <gpiod.h>
const char *gpiochip = "/dev/gpiochip1";
struct gpiod_line_request *line_req = NULL;
#endif

#define diff(a, b) (a > b ? a - b : b - a)

void *set_gpio_val(struct gpio_pin_t *pin) {
    fprintf(stderr, "GPIO thread %d: started.\n", pin->pin);
    while (!pin->should_stop) {
        volatile unsigned int new = pin->new_value;
        if (diff(pin->_current_value, new) >= pin->sensitivity) {
            pin->_current_value = new;
        } else if (pin->pwm == 0) {
            usleep(1000);
            continue;
        }
#ifdef BUILD_PIGPIO
        usleep(pin->_current_value);
#elif defined(BUILD_LIBGPIOD)
        gpiod_line_request_set_value(line_req, pin->pin,
                                     GPIOD_LINE_VALUE_ACTIVE);
        usleep(pin->_current_value);
        gpiod_line_request_set_value(line_req, pin->pin,
                                     GPIOD_LINE_VALUE_INACTIVE);
#else
        usleep(pin->_current_value);
#endif
        usleep(pin->pwm > pin->_current_value ? pin->pwm - pin->_current_value
                                              : 0);
    }
    fprintf(stderr, "GPIO thread %d: stopped.\n", pin->pin);
    pthread_exit(NULL);
}

int gpio_start(struct gpio_pin_t *pins, size_t num_pins) {
    int exit_status = 0;

#ifdef BUILD_PIGPIO
    gpioCfgInterfaces(PI_DISABLE_SOCK_IF | PI_DISABLE_FIFO_IF |
                      PI_LOCALHOST_SOCK_IF);
    int status = gpioInitialise();
    if (status < 0) {
        printf("%d\n", status);
        return -1;
    }
#elif defined(BUILD_LIBGPIOD)
    struct gpiod_chip *chip = gpiod_chip_open(gpiochip);
    if (!chip) {
        fprintf(stderr, "Failed to open chip at %s.\n", gpiochip);
        return -1;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    if (!settings) {
        fprintf(stderr, "Failed to create settings.\n");
        exit_status = -1;
        goto close_chip;
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    if (!line_cfg) {
        fprintf(stderr, "Failed to create line config.\n");
        exit_status = -1;
        goto free_settings;
    }

    unsigned int *offsets = malloc(sizeof(unsigned int) * num_pins);
    for (int i = 0; i < num_pins; i++) {
        offsets[i] = pins[i].pin;
    }
    if (gpiod_line_config_add_line_settings(line_cfg, offsets, num_pins,
                                            settings) < 0) {
        fprintf(stderr, "Failed to apply line settings.\n");
        exit_status = -1;
        goto free_line_cfg;
    }

    line_req = gpiod_chip_request_lines(chip, NULL, line_cfg);
    if (line_req == NULL) {
        fprintf(stderr, "Failed to request lines.\n");
        exit_status = -1;
    }

free_line_cfg:
    free(offsets);
    gpiod_line_config_free(line_cfg);
free_settings:
    gpiod_line_settings_free(settings);
close_chip:
    gpiod_chip_close(chip);
#else
    fprintf(stderr, "Build was compiled without GPIO support.\n");
#endif

    for (int i = 0; i < num_pins; i++) {
        pins[i].should_stop = false;
        pins[i]._current_value = pins[i].new_value;
        pthread_mutex_init(&(pins[i].mtx), NULL);
        pthread_create(&(pins[i].handle), NULL, (void *)set_gpio_val,
                       &(pins[i]));
    }

    return exit_status;
}

void set_gpio_from_scale(struct gpio_pin_t *pin, const float scale) {
    volatile unsigned int new =
        pin->min + (pin->max - pin->min) * (scale * 0.5f + 0.5f);
    new = new < pin->min ? pin->min : new;
    new = new > pin->max ? pin->max : new;
    pin->new_value = new;
}

void gpio_stop(struct gpio_pin_t *pins, size_t num_pins) {
#ifdef BUILD_PIGPIO
    gpioTerminate();
#elif defined(BUILD_LIBGPIOD)
    gpiod_line_request_release(line_req);
#endif
    for (int i = 0; i < num_pins; i++) {
        pins[i].should_stop = true;
    }
    for (int i = 0; i < num_pins; i++) {
        pthread_join(pins[i].handle, NULL);
    }
}
