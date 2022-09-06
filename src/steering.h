#ifndef STEERING_H
#define STEERING_H

typedef struct {
    float speed;
    float angle[2];
} steering_t;


extern int steering_should_stop;


void steering_start (void);

void steer (steering_t movement);

void steering_stop (void);

#endif  // STEERING_H
