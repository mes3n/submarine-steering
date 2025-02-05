#include "gpio.h"

#include <cinttypes>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#ifdef BUILD_PIGPIO
#include <pigpio.h>
#elif defined(BUILD_LIBGPIOD)
#include <gpiod.h>
const char *gpiochip = "/dev/gpiochip1";
struct gpiod_line_request *line_req = NULL;
#endif

int gpio_start(const unsigned int *offsets, size_t num_offsets) {
#ifdef BUILD_PIGPIO
    gpioCfgInterfaces(PI_DISABLE_SOCK_IF | PI_DISABLE_FIFO_IF |
                      PI_LOCALHOST_SOCK_IF);
    int status = gpioInitialise();
    if (status < 0) {
        printf("%d\n", status);
        return -1;
    }
#elif defined(BUILD_LIBGPIOD)
    int exit_status = 0;

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

    if (gpiod_line_config_add_line_settings(line_cfg, offsets, num_offsets,
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
    gpiod_line_config_free(line_cfg);
free_settings:
    gpiod_line_settings_free(settings);
close_chip:
    gpiod_chip_close(chip);
    return exit_status;
#else
    fprintf(stderr, "Build was compiled without GPIO support.\n");
#endif
    return 0;
}

void set_servo_rotation(const int pin, const float scale) {
#ifdef BUILD_PIGPIO
    int rotation = MID_WIDTH + (SERVO_RANGE * 0.5 * scale);

    if (rotation < MIN_WIDTH)
        rotation = MIN_WIDTH;
    if (rotation > MAX_WIDTH)
        rotation = MAX_WIDTH;

    gpioServo(pin, rotation);
#elif defined(BUILD_LIBGPIOD)
    const int rounded = (int)(10.0f * scale + 0.5f);
    const int rotation =
        SERVO_MID + (int)((SERVO_MAX - SERVO_MIN) / 20 * rounded);
    // Toggle on off with SERVO_* micro second interval
    // to turn the servo to min, mid, or max.
    gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_ACTIVE);
    usleep(rotation);
    gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_INACTIVE);
#endif
}

volatile sig_atomic_t stage = 0;
void next_stage(int signum) {
    fprintf(stderr, "Recieved signal... Starting next stage.\n");
    stage++;
}

void calibrate_esc(int pin) {
#ifdef BUILD_PIGPIO
#elif defined(BUILD_LIBGPIOD)
    signal(SIGINT, next_stage);
    fprintf(stderr, "Start sending MAXIMUM signals to ESC.\n");
    while (stage == 0) {
        gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_ACTIVE);
        usleep(ESC_MAX);
        gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_INACTIVE);
        usleep(ESC_PWM - ESC_MAX);
    }
    fprintf(stderr, "Start sending MINIMUM signals to ESC.\n");
    while (stage == 1) {
        gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_ACTIVE);
        usleep(ESC_MIN);
        gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_INACTIVE);
        usleep(ESC_PWM - ESC_MIN);
    }
    fprintf(stderr, "ESC is configured. Listen for affermative beeps.\n");
#endif
}

void set_motor_speed(const int pin, const float scale) {
#ifdef BUILD_PIGPIO
#elif defined(BUILD_LIBGPIOD)
    if (scale < 0.0f)
        return;
    const int speed = (int)(ESC_MIN + (ESC_MAX - ESC_MIN) * scale);
    printf("speed: %d\n", speed);
    gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_ACTIVE);
    usleep(speed);
    gpiod_line_request_set_value(line_req, pin, GPIOD_LINE_VALUE_INACTIVE);
    usleep(ESC_PWM - speed);
#endif
}

void gpio_stop(void) {
#ifdef BUILD_PIGPIO
    gpioTerminate();
#elif defined(BUILD_LIBGPIOD)
    gpiod_line_request_release(line_req);
#endif
}
