#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/socket.h>

#define RECIEVED_DATA_MAX 128

extern volatile char server_should_stop;
extern volatile char server_connected;

struct listen_arg {
    int socket_fd;
    char *recv_data;

    char *handshake_recv;
    char *handshake_send;
};

// create and return socket used for communication
int server_start(int portnum);
// listen to the created socket
void server_listen(struct listen_arg *args);
// close socket used for communication
void server_stop(int server_socket);

#endif // SERVER_H
