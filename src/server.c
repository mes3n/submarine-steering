#include "server.h"
#include "config.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

// A function which manages client connections and passes on their data
void *server_listen(struct server_thread_t *server_thread) {
    while (!server_thread->should_stop) {
        const struct listen_args_t *args = &server_thread->args;
        char handshake_buf[HANDSHAKE_MAX] = {0};
        struct pollfd pfd = {0};

        pfd.fd = server_thread->socket_fd;
        pfd.events = POLLIN;
        for (int rc = 0; rc == 0; rc = poll(&pfd, 1, 1000)) {
            if (server_thread->should_stop) {
                fprintf(stderr, "Stopping server thread.\n");
                pthread_exit(NULL);
            }
            if (rc < 0) {
                fprintf(stderr, "Failed to poll listener socket.\n");
                pthread_exit(NULL);
            }
        }

        if ((pfd.fd = accept(server_thread->socket_fd, NULL, NULL)) < 0) {
            fprintf(stderr, "Could not accept connection.\n");
            continue;
        }
        fprintf(stderr, "Received connection request.\n");

        fcntl(pfd.fd, F_SETFL, fcntl(pfd.fd, F_GETFL) | O_NONBLOCK);

        pfd.events = POLLIN | POLLHUP;
        while (!server_thread->should_stop) {
            int rc = poll(&pfd, 1, 1000);
            if (rc == 0)
                continue;
            if (rc < 0) {
                fprintf(stderr, "Failed to poll connection socket.\n");
                break;
            }
            if (pfd.revents & POLLHUP)
                break;
            if (!server_thread->connected) {
                int n = recv(pfd.fd, handshake_buf, HANDSHAKE_MAX, 0);
                if (n <= 0) {
                    if (n < 0)
                        fprintf(stderr, "Could not recieve data.\n");
                    break;
                }
                if (strncmp(handshake_buf, args->handshake_recv, n) != 0) {
                    fprintf(stderr,
                            "Handshake failed. '%s' and '%s' don't match.\n",
                            handshake_buf, args->handshake_recv);
                    break;
                }
                // Recieved handshake accepted -> Send one back
                send(pfd.fd, args->handshake_send, HANDSHAKE_MAX, 0);
                server_thread->connected = true;

                fprintf(stderr, "Client connected.\n");
                continue;
            }
            pthread_mutex_lock(&server_thread->data_mtx);
            memset(args->recv_data, 0x0, RECIEVED_DATA_MAX);
            int n = recv(pfd.fd, args->recv_data, RECIEVED_DATA_MAX, 0);
            if (n <= 0) {
                if (n < 0)
                    fprintf(stderr, "Could not recieve data.\n");
                pthread_mutex_unlock(&server_thread->data_mtx);
                break;
            }
            send(pfd.fd, args->recv_data, RECIEVED_DATA_MAX, 0);
            pthread_mutex_unlock(&server_thread->data_mtx);
            pthread_cond_signal(&server_thread->data_cond);
        }

        server_thread->connected = false;
        shutdown(pfd.fd, SHUT_RDWR);
        close(pfd.fd);
        printf("Server connection stopped.\n");
    }
    pthread_exit(NULL);
}

int server_start(struct server_thread_t *server_thread, int port) {
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    // set our address to any interface
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (fd < 0) {
        fprintf(stderr, "Couldn't create socket.\n");
        return -1;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        fprintf(stderr, "Failed to set socket options.\n");
        return -1;
    }

    if (bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0) {
        fprintf(stderr, "Couldn't bind socket.\n");
        close(fd);
        return -1;
    }

    if (listen(fd, 0) < 0) {
        fprintf(stderr, "Couldn't listen on socket.\n");
        close(fd);
        return -1;
    }

    pthread_mutex_init(&server_thread->data_mtx, NULL);
    pthread_cond_init(&server_thread->data_cond, NULL);

    server_thread->socket_fd = fd;
    printf("Connection established on port %d.\n", port);

    server_thread->connected = false;
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
    pthread_mutex_destroy(&server_thread->data_mtx);
    pthread_cond_destroy(&server_thread->data_cond);
    server_thread->should_stop = true;

    pthread_join(server_thread->handle, NULL);
    shutdown(server_thread->socket_fd, SHUT_RDWR);
    close(server_thread->socket_fd);
}
