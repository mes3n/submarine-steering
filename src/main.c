#include "config.h"
#include "gpio.h"
#include "server.h"

#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *help_message =
    "Usage: %s [-c CONFIG_FILE] [-h] ...\nSimple program to which communicates "
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
    "  -h                 view this help menu\n";

// Union used to receive raw data
union data_t {
    struct {
        char fn;
        char padding[3];
        union fn_var_t {
            float rotation;
            float speed;
        } var;
    } cmd;
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
    struct config_t config;
    reset_config(&config);
    while ((opt = getopt(argc, argv, "hc:p:R:S:x:y:m:")) != (char)0xff) {
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
        default:
            fprintf(stderr, help_message, argv[0], config_file);
            return 1;
        }
    }

    if (read_from(config_file, &config, true) < 0) {
        fprintf(stderr, "Failed to read from config.\n");
        return 1;
    }

    fprintf(stderr, "Running with:\n");
    fprintf(stderr, "  - port = %d\n", config.port);
    fprintf(stderr, "  - handshake_recv = %s\n", config.handshake_recv);
    fprintf(stderr, "  - handshake_send = %s\n", config.handshake_send);
    fprintf(stderr, "  - steer_x_pin = %d\n", config.steer_x_pin);
    fprintf(stderr, "  - steer_y_pin = %d\n", config.steer_y_pin);
    fprintf(stderr, "  - motor_ctrl_pin = %d\n", config.motor_ctrl_pin);

    struct {
        struct gpio_pin_t steer_x;
        struct gpio_pin_t steer_y;
        struct gpio_pin_t motor_ctrl;
    } gpios;

    gpios.steer_x.pin = config.steer_x_pin;
    gpios.steer_x.pwm = 0;
    gpios.steer_x.min = 500;
    gpios.steer_x.max = 2500;
    gpios.steer_x.new_value = 1500;
    gpios.steer_x.sensitivity = 100;

    gpios.steer_y.pin = config.steer_y_pin;
    gpios.steer_y.pwm = 0;
    gpios.steer_y.min = 500;
    gpios.steer_y.max = 2500;
    gpios.steer_y.new_value = 1500;
    gpios.steer_y.sensitivity = 100;

    gpios.motor_ctrl.pin = config.motor_ctrl_pin;
    gpios.motor_ctrl.pwm = 20000;
    gpios.motor_ctrl.min = 1000;
    gpios.motor_ctrl.max = 2000;
    gpios.motor_ctrl.new_value = 1000;
    gpios.motor_ctrl.sensitivity = 50;

    if (gpio_start((struct gpio_pin_t *)&gpios, 3) < 0) {
        fprintf(stderr, "GPIO failed to initialise.\n");
        return 2;
    }

    signal(SIGINT, sig_handle);
    signal(SIGTERM, sig_handle);

    union data_t data = {0};
    struct server_thread_t server_thread = {
        .args = {
            .recv_data = data.raw,
            .handshake_recv = config.handshake_recv,
            .handshake_send = config.handshake_send,
        }};
    if (server_start(&server_thread, config.port) < 0) {
        fprintf(stderr, "Server failed to initialise.\n");
        return 3;
    }

    while (!stop) {
        pthread_mutex_lock(&server_thread.data_mtx);
        for (;;) {
            if (stop)
                goto exit;
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 1;
            int rc = pthread_cond_timedwait(&server_thread.data_cond,
                                            &server_thread.data_mtx, &ts);
            if (rc == 0)
                break;
        }

        char fn = data.cmd.fn;
        union fn_var_t var = data.cmd.var;
        data.cmd.fn = 0x0;

        pthread_mutex_unlock(&server_thread.data_mtx);

        switch (fn) {
        case 'x':
            set_gpio_from_scale(&gpios.steer_x, var.rotation);
            break;
        case 'y':
            set_gpio_from_scale(&gpios.steer_y, var.rotation);
            break;
        case 'm':
            set_gpio_from_scale(&gpios.motor_ctrl, (var.speed - 0.5f) * 2.f);
            break;
        default:
            break;
        }
    }

exit:
    gpio_stop((struct gpio_pin_t *)&gpios, 3);
    server_stop(&server_thread);

    return 0;
}
