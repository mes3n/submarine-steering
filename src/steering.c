#include "steering.h"

#include "servo.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>


int steering_start (void) {
    if (init_servos() < 0) return -1;
    memset(&movement, 0, sizeof(movement));
}

void steer (void) {
    while (!steering_should_stop) {
        printf("speed: %.5f\n", movement.speed);
        printf("%.5f, %.5f\n", movement.angle[0], movement.angle[1]);

        set_servo_rotation(0, movement.angle[0]);

        usleep(10000);
    }
}

void steering_stop (void) {
    steering_should_stop = 1;
    stop_servos();
}
