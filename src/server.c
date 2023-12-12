#include "server.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

#include <unistd.h> // close, sleep

int server_start(int portnum) {
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr =
        htonl(INADDR_ANY);            // set our address to any interface
    server.sin_port = htons(portnum); // set the server port number

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Couldn't start socket.\n");
        return -1;
    }

    while (bind(server_socket, (struct sockaddr *)&server,
                sizeof(struct sockaddr)) < 0) {
        printf("Couldn't bind socket, trying again in 3 seconds.\n");
        sleep(3); // wait n seconds before trying again
    }

    printf("Connection established on port %d.\n", portnum);
    return server_socket;
}

void server_listen(struct listen_arg *args) {
    struct sockaddr_in destination;
    socklen_t destination_size = sizeof(struct sockaddr_in);

    while (!server_should_stop) {
        listen(args->socket_fd, 0);
        // should maybe start a new thread for each accept (if more than one is
        // needed?)
        int conn_sock = accept(args->socket_fd, (struct sockaddr *)&destination,
                               &destination_size);
        printf("Connection from %s\n", inet_ntoa(destination.sin_addr));

        if (!(recv(conn_sock, args->recv_data, RECIEVED_DATA_MAX, 0) > 0)) {
            goto close_conn;
        }
        if (strncmp(args->recv_data, args->handshake_recv, RECIEVED_DATA_MAX) !=
            0) { // Recieved handshake fails
            goto close_conn;
        } else { // Recieved handshake accepted -> Send one back
            send(conn_sock, args->handshake_send, HANDSHAKE_MAX, 0);
        }

        memset(args->recv_data, 0, RECIEVED_DATA_MAX);

        server_connected = 1;
        while (!server_should_stop &&
               recv(conn_sock, args->recv_data, RECIEVED_DATA_MAX, 0) > 0) {
            // send(conn_sock, args->recv_data, 12, 0);
            usleep(10 * 1000);
        }
        server_connected = 0;

    close_conn:
        close(conn_sock);
        printf("Server listener stopped.\n");
    }
}

void server_stop(int server_socket) {
    shutdown(server_socket, SHUT_RDWR);
    close(server_socket);

    server_should_stop = 1;
}
