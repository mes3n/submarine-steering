#ifndef SERVO_H
#define SERVO_H

#define MIN_WIDTH 500
#define MID_WIDTH 1500
#define MAX_WIDTH 2500

#define SERVO_RANGE 2000


int init_servos (void);

void set_servo_rotation (int i, float scale);

void stop_servos (void);

#endif  // SERVO_H