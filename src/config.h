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

/**
 * Struct containing application configuration
 */
struct config_t {
    /// Used to mark which fields have been set using bits
    unsigned fields;

    int port;
    char handshake_recv[HANDSHAKE_MAX];
    char handshake_send[HANDSHAKE_MAX];

    int steer_x_pin;
    int steer_y_pin;
    int motor_ctrl_pin;
};

/**
 * Load configuration from a configuration file
 *
 * @param path A string describing the path of the configuration file
 * @param config A pointer to a config_t object to load configuration into
 * @param use_default Load hard coded default values if none other were set
 * @returns Returns an integer 0 for success otherwise negative
 */
int read_from(char *path, struct config_t *config, bool use_default);

/**
 * Reset configuration struct object be zeroing its fields
 *
 * @params config A pointer to a config_t object to be reset
 */
void reset_config(struct config_t *config);

#endif // CONFIG_H
