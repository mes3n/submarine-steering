#ifndef CONNECTION_H
#define CONNECTION_H

#include <arpa/inet.h>
#include <sys/socket.h>

#define PORTNUM 2300
#define RECIEVEC_DATA_MAX 128


extern int server_should_stop;
extern char* server_data;

int server_start (void);  // create and return socket used for communication

void server_listen (int server_socket);  // listen to the created socket

void server_stop (int server_socket);  // close socket used for communication

#endif  // CONNECTION_H
