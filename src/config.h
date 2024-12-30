#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define HANDSHAKE_MAX 16

#define DEFAULT_port 2300
#define FIELD_N_port 0
#define FORMAT_port "d"
#define IS_PTR_port 0

#define DEFAULT_handshake_recv ""
#define FIELD_N_handshake_recv 1
#define FORMAT_handshake_recv "s"
#define IS_PTR_handshake_recv 1

#define DEFAULT_handshake_send ""
#define FIELD_N_handshake_send 2
#define FORMAT_handshake_send "s"
#define IS_PTR_handshake_send 1

#define DEFAULT_steer_x_pin 2
#define FIELD_N_steer_x_pin 3
#define FORMAT_steer_x_pin "d"
#define IS_PTR_steer_x_pin 0

#define DEFAULT_steer_y_pin 3
#define FIELD_N_steer_y_pin 4
#define FORMAT_steer_y_pin "d"
#define IS_PTR_steer_y_pin 0

#define DEFAULT_motor_ctrl_pin 4
#define FIELD_N_motor_ctrl_pin 5
#define FORMAT_motor_ctrl_pin "d"
#define IS_PTR_motor_ctrl_pin 0

struct config_t {
    unsigned fields;

    int port;
    char handshake_recv[HANDSHAKE_MAX];
    char handshake_send[HANDSHAKE_MAX];

    int steer_x_pin;
    int steer_y_pin;
    int motor_ctrl_pin;
};

int read_from(char *path, struct config_t *config, bool use_default);

int reset_config(struct config_t *config);

#endif // CONFIG_H
