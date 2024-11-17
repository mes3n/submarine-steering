#include "config.h"
#include "gpio.h"
#include "server.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>

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
    stop++;
    fprintf(stderr, "Recieved stop signal %d. Exiting...\n", signum);
    if (stop > 1) {
        fprintf(stderr, "Recieved %d stop signals. Forcing exit...\n", stop);
        exit(stop);
    }
}

int main(int argc, char **argv) {
    char opt, *config_file = "/etc/steering.conf";
    while ((opt = getopt(argc, argv, "c:h")) != -1) {
        switch (opt) {
        case 'c':
            config_file = optarg;
            break;
        case 'h':
        default:
            fprintf(stderr, "Usage: %s [-h] [-c CONFIG]\n", argv[0]);
            return -1;
        }
    }

    struct config_t config;
    if (read_from(config_file, &config, 1) < 0) {
        fprintf(stderr, "Failed to read from config.\n");
        return -1;
    }

    fprintf(stderr, "Running with:\n");
    fprintf(stderr, "  - port = %d\n", config.port);
    fprintf(stderr, "  - handshake_recv = %s\n", config.handshake_recv);
    fprintf(stderr, "  - handshake_send = %s\n", config.handshake_send);

    if (gpio_start() < 0) {
        fprintf(stderr, "GPIO failed to initialise.\n");
        return -1;
    }

    signal(SIGINT, sig_handle);
    signal(SIGTERM, sig_handle);

    union data_t data;
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
            set_servo_rotation(GPIO_STEER_X, data.move.steerx);
            fprintf(stderr, "\rspeed: %.5f - steer: %.5f, %.5f",
                    data.move.speed, data.move.steerx, data.move.steery);
        }
        usleep(10 * 1000);
    }

    gpio_stop();
    server_stop(&server_thread);

    return 0;
}
