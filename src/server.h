#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/socket.h>

#define PORTNUM 2300
#define RECIEVED_DATA_MAX 128

extern volatile char server_should_stop;
extern volatile char server_connected;

struct listen_arg {
    int socket_fd;
    char *recv_data;
};

int server_start(void); // create and return socket used for communication
// listen to the created socket
void server_listen(struct listen_arg *args);
void server_stop(int server_socket); // close socket used for communication

#endif // SERVER_H
