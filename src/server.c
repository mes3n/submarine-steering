#include "server.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include <unistd.h> // close, sleep

void server_listen(struct server_thread_t *server_thread) {
    struct sockaddr_in destination;
    socklen_t destination_size = sizeof(struct sockaddr_in);

    const struct listen_args_t *args = &server_thread->args;
    while (!server_thread->should_stop) {
        listen(server_thread->socket_fd, 0);
        int conn_sock =
            accept(server_thread->socket_fd, (struct sockaddr *)&destination,
                   &destination_size);
        printf("Connection from %s\n", inet_ntoa(destination.sin_addr));

        if (recv(conn_sock, args->recv_data, RECIEVED_DATA_MAX, 0) < 0) {
            fprintf(stderr, "Could not recieve data.\n");
            goto close_conn;
        }
        if (strncmp(args->recv_data, args->handshake_recv, RECIEVED_DATA_MAX) !=
            0) { // Recieved handshake fails
            fprintf(stderr, "Handshake failed. %s and %s don't match.\n",
                    args->recv_data, args->handshake_recv);
            goto close_conn;
        } else { // Recieved handshake accepted -> Send one back
            send(conn_sock, args->handshake_send, HANDSHAKE_MAX, 0);
        }

        memset(args->recv_data, 0, RECIEVED_DATA_MAX);

        server_thread->connected = 1;
        while (!server_thread->should_stop &&
               recv(conn_sock, args->recv_data, RECIEVED_DATA_MAX, 0) > 0) {
            send(conn_sock, args->recv_data, RECIEVED_DATA_MAX, 0);
            usleep(10 * 1000);
        }
        server_thread->connected = 0;

    close_conn:
        close(conn_sock);
        printf("Server connection stopped.\n");
    }
}

int server_start(int port, struct server_thread_t *server_thread) {
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    // set our address to any interface
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "Couldn't create socket.\n");
        return -1;
    }

    int i = 0;
    while (bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "Couldn't bind socket.\n");
        if (i > 9) {
            fprintf(stderr, "Timed out after %d attmepts.\n", i);
            close(fd);
            return -1;
        }
        fprintf(stderr, "Trying again in %d seconds.\n", i);
        sleep(i); // wait n seconds before trying again
        i++;
    }

    server_thread->socket_fd = fd;
    printf("Connection established on port %d.\n", port);

    server_thread->should_stop = false;
    if (pthread_create(&(server_thread->handle), NULL, (void *)server_listen,
                       server_thread) != 0) {
        fprintf(stderr, "Failed to start server listener.\n");
        close(fd);
        return -1;
    }

    return 0;
}

void server_stop(struct server_thread_t *server_thread) {
    shutdown(server_thread->socket_fd, SHUT_RDWR);
    close(server_thread->socket_fd);

    server_thread->should_stop = true;

    pthread_join(server_thread->handle, NULL);
}
