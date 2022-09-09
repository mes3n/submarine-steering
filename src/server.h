#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <sys/socket.h>

#define PORTNUM 2300
#define RECIEVED_DATA_MAX 128


extern unsigned char server_should_stop;
extern unsigned char* server_data;

int server_start (void);  // create and return socket used for communication

void server_listen (int server_socket);  // listen to the created socket

void server_stop (int server_socket);  // close socket used for communication

#endif  // SERVER_H
