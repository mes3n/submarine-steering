#include "server.h"
#include "steering.h"

#include <pthread.h>

#include <unistd.h>
#include <stdio.h>

int server_should_stop = 0;
int steering_should_stop = 0;
char* server_data;


int main(int argc, char **argv) {

    int server_socket = server_start();
    if (server_socket < 0) {
        printf("Socket failed to start.\n");
        return -1; 
    }
    steering_start();

    pthread_t server_thread, steering_thread;
    pthread_create(&server_thread, NULL, server_listen, server_socket);
    pthread_create(&steering_thread, NULL, steer, "hello world");

    for (int i = 0; i <= 6; i++) {
        printf("server data:\n%s\n", server_data);
        sleep(1);
    }

    // server_should_stop = 1;
    server_stop(server_socket);
    steering_stop();

    pthread_join(steering_thread, NULL);
    pthread_join(server_thread, NULL);
    
}