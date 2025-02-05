#include "config.h"
#include "gpio.h"
#include "server.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *help_message =
    "Usage: %s [-c CONFIG_FILE] [-h] ...\nSimple program to which comminucates "
    "with gpio pins based on information transferred through a basic socket "
    "protocol.\nEvery value has a default and can otherwise be set from a "
    "specified config file (defaul: %s) or command line arguments (which are "
    "prioritized).\n\nAll arguments are optional.\n"
    "  -c CONFIG_FILE     load configuration from a config file, can be "
    "combined and overriden by other arguments\n"
    "  -p PORT            port the program will use for communication\n"
    "  -R HANDSHAKE_RECV  handshake string program expects to recieve\n"
    "  -S HANDSHAKE_SEND  handshake string program will respond with\n"
    "  -x STEER_X_PIN     the gpio pin used to control horizontal movement\n"
    "  -y STEER_Y_PIN     the gpio pin used to control vertical movement\n"
    "  -m MOTOR_CTRL_PIN  the gpio pin used to control motor speed\n"
    "  -E                 start the interactive ESC configuration using ^C\n"
    "  -h                 view this help menu\n";

union data_t {
    struct {
        float speed;
        float steerx;
        float steery;
    } move;
    char raw[RECIEVED_DATA_MAX];
};

volatile sig_atomic_t stop = 0;
void sig_handle(int signum) {
    fprintf(stderr, "Recieved stop signal %d. Exiting...\n", signum);
    if (stop++ > 1) {
        fprintf(stderr, "Recieved %d stop signals. Forcing exit...\n", stop);
        exit(stop);
    }
}

int main(int argc, char **argv) {
    char opt, *config_file = "/etc/steering.conf";
    bool configure_esc = false;
    struct config_t config;
    reset_config(&config);
    while ((opt = getopt(argc, argv, "Ehc:p:R:S:x:y:m:")) != (char)0xff) {
        switch (opt) {
        case 'c':
            config_file = optarg;
            break;
        case 'p':
            sscanf(optarg, "%d", &config.port);
            config.fields |= 0x1 << FIELD_N_port;
            break;
        case 'R':
            strncpy(config.handshake_recv, optarg,
                    sizeof(config.handshake_recv) - 1);
            config.fields |= 0x1 << FIELD_N_handshake_recv;
            break;
        case 'S':
            strncpy(config.handshake_send, optarg,
                    sizeof(config.handshake_send) - 1);
            config.fields |= 0x1 << FIELD_N_handshake_send;
            break;
        case 'x':
            sscanf(optarg, "%d", &config.steer_x_pin);
            config.fields |= 0x1 << FIELD_N_steer_x_pin;
            break;
        case 'y':
            sscanf(optarg, "%d", &config.steer_y_pin);
            config.fields |= 0x1 << FIELD_N_steer_y_pin;
            break;
        case 'm':
            sscanf(optarg, "%d", &config.motor_ctrl_pin);
            config.fields |= 0x1 << FIELD_N_motor_ctrl_pin;
            break;
        case 'h':
            fprintf(stderr, help_message, argv[0], config_file);
            return 0;
        case 'E':
            fprintf(stderr, "Will calibrate ESC then exit...\n");
            configure_esc = true;
            break;
        }
    }

    if (read_from(config_file, &config, true) < 0) {
        fprintf(stderr, "Failed to read from config.\n");
        return -1;
    }

    fprintf(stderr, "Running with:\n");
    fprintf(stderr, "  - port = %d\n", config.port);
    fprintf(stderr, "  - handshake_recv = %s\n", config.handshake_recv);
    fprintf(stderr, "  - handshake_send = %s\n", config.handshake_send);
    fprintf(stderr, "  - steer_x_pin = %d\n", config.steer_x_pin);
    fprintf(stderr, "  - steer_y_pin = %d\n", config.steer_y_pin);
    fprintf(stderr, "  - motor_ctrl_pin = %d\n", config.motor_ctrl_pin);

    const unsigned int offsets[] = {config.steer_x_pin, config.steer_y_pin,
                                    config.motor_ctrl_pin};
    if (gpio_start(offsets, sizeof(offsets) / sizeof(offsets[0])) < 0) {
        fprintf(stderr, "GPIO failed to initialise.\n");
        return -1;
    }

    if (configure_esc) {
        calibrate_esc(config.motor_ctrl_pin);
        return 0;
    }

    signal(SIGINT, sig_handle);
    signal(SIGTERM, sig_handle);

    union data_t data = {0};
    struct server_thread_t server_thread;
    server_thread.args.recv_data = data.raw;
    server_thread.args.handshake_recv = config.handshake_recv;
    server_thread.args.handshake_send = config.handshake_send;
    if (server_start(config.port, &server_thread) < 0) {
        fprintf(stderr, "Server failed to initialise.\n");
        return -1;
    }

    while (!stop) { // mainloop
        if (server_thread.connected) {
            set_servo_rotation(config.steer_x_pin, data.move.steerx);
            set_servo_rotation(config.steer_y_pin, data.move.steery);
            set_motor_speed(config.motor_ctrl_pin, data.move.speed);
            fprintf(stderr, "speed: %.5f - steer: %.5f, %.5f\n",
                    data.move.speed, data.move.steerx, data.move.steery);
        }
        usleep(100 * 1000);
    }

    gpio_stop();
    server_stop(&server_thread);

    return 0;
}
