#include "server.h"
#include "gpio.h"

#include <pthread.h>
#include <stdlib.h>
#include <signal.h>

#include <stddef.h>
#include <string.h>

#include <unistd.h>  // TODO: sleep change to something more precise
#include <stdio.h>

char* server_data = NULL;
volatile char server_should_stop = 0;
volatile char server_connected = 0;

volatile sig_atomic_t stop = 0;

void sig_handle(int signum) {
    printf("Recieved stop signal 2. Exiting...\n");
    stop = 1;
}

int main(int argc, char** argv) {

    steering_t movement;
    if (gpio_start(&movement) < 0) {
        printf("Gpio failed to initialise.\n");
        return -1;
    }
    int server_socket = server_start();
    if (server_socket < 0) {
        printf("Socket failed to initialise.\n");
        return -1;
    }

    pthread_t server_thread;
    pthread_create(&server_thread, NULL, server_listen, server_socket);
    
    signal(SIGINT, sig_handle);
    while (!stop) {  // mainloop

        if (server_connected && memcmp(&movement, server_data, sizeof(steering_t)) != 0) {

            for (int i = 0; i < sizeof(movement); i++) {
                printf("%d, ", server_data[i]);
            }
            printf("\n");

            // MMMMMMM WE LOVE MEMORY SAFETY
            if (memcmp(&movement.speed, &server_data[offsetof(steering_t, speed)], sizeof(movement.speed)) != 0) {  // spped was changed
                
            }
            if (memcmp(&movement.steerx, &server_data[offsetof(steering_t, steerx)], sizeof(movement.steerx)) != 0) {  // anglex was changed
                set_servo_rotation(GPIO_STEER_X, movement.steerx);
                set_servo_rotation(GPIO_STEER_X, movement.steerx);
            }
            if (memcmp(&movement.steery, &server_data[offsetof(steering_t, steery)], sizeof(movement.steery)) != 0) {  // angley was changed
                
            }

            memcpy(&movement, server_data, sizeof(movement));

            printf("speed: %.5f\n", movement.speed);
            printf("%.5f, %.5f\n", movement.steerx, movement.steery);
        }

        usleep(10 * 1000);
    }

    gpio_stop();

    server_stop(server_socket);
    pthread_join(server_thread, NULL);

    return 0;
}