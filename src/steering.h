#ifndef STEERING_H
#define STEERING_H


typedef struct {
    double speed;
    double angle[2];
} steering_t;

extern unsigned char steering_should_stop;
extern steering_t movement;


void steering_start (void);

void steer (void);

void steering_stop (void);

#endif  // STEERING_H
