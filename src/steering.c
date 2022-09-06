#include "steering.h"

#include <unistd.h>
#include <stdio.h>

void steering_start (void) {

}

void steer (steering_t movement) {
    while (!steering_should_stop) {
        printf("%2f\n", movement.speed);
        printf("%2f, %2f\n", movement.angle[0], movement.angle[1]);
        sleep(2);
    }
}

void steering_stop (void) {
    steering_should_stop = 1;
}
