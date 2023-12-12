#ifndef CONFIG_H
#define CONFIG_H

#define HANDSHAKE_MAX 16

struct Config {
    unsigned fields;

    int port;
    char handshake_recv[HANDSHAKE_MAX];
    char handshake_send[HANDSHAKE_MAX];
};

int read_from(char *path, struct Config *config, int else_use_default);

#endif // CONFIG_H
