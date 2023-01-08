#include "server.h"
#include "steering.h"

#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>  // TODO: sleep change to something more precise
#include <stdio.h>

unsigned char server_should_stop = 0;
unsigned char* server_data;

unsigned char steering_should_stop = 0;
steering_t movement;

int main(int argc, char **argv) {

    int server_socket = server_start();
    if (server_socket < 0) {
        printf("Socket failed to start.\n");
        return -1; 
    }
    if (steering_start() < 0) {
        printf("Steering failed initialise.\n");
        return -1;
    }

    pthread_t server_thread, steering_thread;
    pthread_create(&server_thread, NULL, server_listen, server_socket);
    pthread_create(&steering_thread, NULL, steer, 0);

    for (int i = 0; i <= 10000; i++) {  // mainloop
        for (int i = 0; i < sizeof(movement); i++) {
            printf("%d, ", server_data[i]);
        }
        printf("\n");
        memcpy(&movement, server_data, sizeof(movement));
        usleep(10000);
    }

    server_stop(server_socket);
    steering_stop();

    pthread_join(steering_thread, NULL);
    pthread_join(server_thread, NULL);
    
}