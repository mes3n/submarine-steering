#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <sys/socket.h>

#define RECIEVED_DATA_MAX 128

struct listen_args_t {
    char *recv_data;

    char *handshake_recv;
    char *handshake_send;
};

struct server_thread_t {
    int socket_fd;
    pthread_t handle;

    atomic_bool should_stop;
    atomic_bool connected;

    struct listen_args_t args;
};

// create and return socket used for communication
int server_start(int port, struct server_thread_t *server_thread);
// close socket used for communication
void server_stop(struct server_thread_t *server_thread);

#endif // SERVER_H
