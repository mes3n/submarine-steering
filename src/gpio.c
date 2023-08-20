#include "gpio.h"

#include <pigpio.h>
#include <string.h>

#include <stdio.h>

int gpio_start(steering_t* movement) {  // TODO: set this to gpio pins
    gpioCfgInterfaces(PI_DISABLE_SOCK_IF | PI_DISABLE_FIFO_IF | PI_LOCALHOST_SOCK_IF);
    int status = gpioInitialise();
    if (status < 0) {
        printf("%d\n", status); 
        return -1;
    }

    memset(movement, 0, sizeof(steering_t));
    return 0;
}

void set_servo_rotation(int pin, float scale) {

    int rotation = MID_WIDTH + (SERVO_RANGE * 0.5 * scale);

    if (rotation < MIN_WIDTH) rotation = MIN_WIDTH;
    if (rotation > MAX_WIDTH) rotation = MAX_WIDTH;

    gpioServo(pin, rotation);
}

void gpio_stop(void) {
    gpioTerminate();
    printf("Pigpio was stopped.\n");
}
