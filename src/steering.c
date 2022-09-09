#include "steering.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

void steering_start (void) {
    memset(&movement, 0, sizeof(movement));
}

void steer (void) {
    while (!steering_should_stop) {
        printf("speed: %.2f\n", movement.speed);
        printf("%.2f, %.2f\n", movement.angle[0], movement.angle[1]);
        sleep(1);
    }
}

void steering_stop (void) {
    steering_should_stop = 1;
}
