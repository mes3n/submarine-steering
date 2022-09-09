#include "server.h"
#include "steering.h"

#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>  // sleep change to something more precise
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
    steering_start();

    pthread_t server_thread, steering_thread;
    pthread_create(&server_thread, NULL, server_listen, server_socket);
    pthread_create(&steering_thread, NULL, steer, 0);

    for (int i = 0; i <= 500; i++) {  // mainloop
        // printf("i: %s", server_data);
        for (int i = 0; i < RECIEVED_DATA_MAX; i++) {
            if (server_data[i] == 's' && i < RECIEVED_DATA_MAX - 1) {
                movement.speed = atof(&server_data[i+1]);
            }
            if (server_data[i] == 'x' && i < RECIEVED_DATA_MAX - 1) {
                movement.angle[0] = atof(&server_data[i+1]);
            }
            if (server_data[i] == 'y' && i < RECIEVED_DATA_MAX - 1) {
                movement.angle[1] = atof(&server_data[i+1]);
            }
        }
        memset(server_data, 0, RECIEVED_DATA_MAX);
        sleep(1);
    }

    server_stop(server_socket);
    steering_stop();

    pthread_join(steering_thread, NULL);
    pthread_join(server_thread, NULL);
    
}