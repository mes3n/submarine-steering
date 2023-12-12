#include "config.h"
#include "gpio.h"
#include "server.h"

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#include <stddef.h>
#include <string.h>

#include <stdio.h>
#include <unistd.h> // TODO: sleep change to something more precise

union data_t {
    steering_t move;
    char raw[RECIEVED_DATA_MAX];
};

volatile sig_atomic_t stop = 0;
void sig_handle(int signum) {
    printf("Recieved stop signal %d. Exiting...\n", signum);
    stop = 1;
}

volatile char server_should_stop = 0;
volatile char server_connected = 0;

int main(int argc, char **argv) {

    struct Config config;
    if (read_from("main.conf", &config, 1) < 0) {
        printf("Failed to read from config.\n");
        return -1;
    }
    printf("Running with:\n"
           "\tport = %d\n"
           "\thandshake_recv = %s\n"
           "\thandshake_send = %s\n",
           config.port, config.handshake_send, config.handshake_recv);

    union data_t data;
#ifndef NO_PIGPIO
    if (gpio_start() < 0) {
        printf("Gpio failed to initialise.\n");
        return -1;
    }
#else
    printf("Build was compiled without Gpio.\n");
#endif

    int server_socket = server_start(config.port);
    if (server_socket < 0) {
        printf("Socket failed to initialise.\n");
        return -1;
    }

    pthread_t server_thread;
    // struct listen_arg args =
    pthread_create(&server_thread, NULL, (void *)server_listen,
                   &(struct listen_arg){server_socket, data.raw});

    signal(SIGINT, sig_handle);
    signal(SIGTERM, sig_handle);
    while (!stop) { // mainloop

        if (server_connected) {

            // for (int i = 0; i < sizeof(movement); i++) {
            //   printf("%d, ", server_data[i]);
            // }
            // printf("\n");

#ifndef NO_PIGPIO
            set_servo_rotation(GPIO_STEER_X, data.move.steerx);
            set_servo_rotation(GPIO_STEER_X, data.move.steerx);
#endif

            printf("speed: %.5f\n", data.move.speed);
            printf("%.5f, %.5f\n", data.move.steerx, data.move.steery);
        }

        usleep(10 * 1000);
    }

#ifndef NO_PIGPIO
    gpio_stop();
#endif

    server_stop(server_socket);
    pthread_join(server_thread, NULL);

    return 0;
}
