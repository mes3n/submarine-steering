#include "server.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>  // close, sleep


int server_start (void) {

    server_data = (unsigned char*)malloc(RECIEVED_DATA_MAX);
    memset(server_data, 0, RECIEVED_DATA_MAX);

    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);  // set our address to any interface
    server.sin_port = htons(PORTNUM);  // set the server port number   

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Couldn't start socket.\n");
        return -1;
    }

    while (bind(server_socket, (struct sockaddr*)&server, sizeof(struct sockaddr)) < 0) {
        printf("Couldn't bind socket, trying again in 3 seconds.\n");
        sleep(3);  // wait n seconds before trying again
    }

    printf("Connection established on port %d.\n", PORTNUM);
    return server_socket;
}
 
void server_listen (int server_socket) {
    while (!server_should_stop) {
        struct sockaddr_in destination;
        socklen_t destination_size = sizeof(struct sockaddr_in);

        listen(server_socket, 0);
        // should maybe start a new thread for each accept
        int connection_socket = accept(server_socket, (struct sockaddr*)&destination, &destination_size);  
        printf("Connection from %s\n", inet_ntoa(destination.sin_addr));
        while (!server_should_stop && recv(connection_socket, server_data, RECIEVED_DATA_MAX, 0) > 0) {
            send(connection_socket, "Hello Client!", 14, 0);
            // sleep(1);
        }
        close(connection_socket);
        printf("Server listener stopped.\n");
    }
}

void server_stop (int server_socket) {
    server_should_stop = 1;
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);
}
