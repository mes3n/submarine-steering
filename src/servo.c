
#include "servo.h"

#include <pigpio.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static int activeGPIO[2] = {0, 0};

static void stop(int signum) {
   gpioTerminate();
}

int init_servos (void) {  // TODO: set this to gpio pins
   if (gpioInitialise() < 0) return -1;
   // gpioSetSignalFunc(SIGINT, stop);
   activeGPIO[0] = 17;
}

void set_servo_rotation (int i, float scale) {
   
   int rotation = MID_WIDTH + (SERVO_RANGE * 0.5 * scale);

   if (rotation < MIN_WIDTH) rotation = MIN_WIDTH;
   if (rotation > MAX_WIDTH) rotation = MAX_WIDTH;

   gpioServo(activeGPIO[0], rotation);
}

void stop_servos (void) {
   gpioTerminate();
}
