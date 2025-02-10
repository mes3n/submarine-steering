#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

#define RECIEVED_DATA_MAX 128

/**
 * Struct ...
 */
struct listen_args_t {
    char *recv_data;
    char *handshake_recv;
    char *handshake_send;
};

/**
 * Struct ...
 */
struct server_thread_t {
    int socket_fd;
    pthread_t handle;
    pthread_mutex_t data_mtx;
    pthread_cond_t data_cond;

    atomic_bool should_stop;
    atomic_bool connected;

    struct listen_args_t args;
};

/**
 * Start submarine transport server using a server_thread_t object and the
 * port it should bind to
 *
 * @param server_thread A pointer to a server_thread_t object with the args
 * field set
 * @param port The port the server should bind to
 * @return An integer 0 for success otherwise negative
 */
int server_start(struct server_thread_t *server_thread, int port);

/**
 * Stop a start server thread object
 *
 * @param server_thread The server_thread_t object to be stopped
 */
void server_stop(struct server_thread_t *server_thread);

#endif // SERVER_H
