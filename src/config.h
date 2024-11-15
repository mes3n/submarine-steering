#ifndef CONFIG_H
#define CONFIG_H

#define HANDSHAKE_MAX 16

#define DEFAULT_port 2300
#define FIELD_N_port 1
#define FORMAT_port "d"
#define IS_PTR_port 0

#define DEFAULT_handshake_recv ""
#define FIELD_N_handshake_recv 2
#define FORMAT_handshake_recv "s"
#define IS_PTR_handshake_recv 1

#define DEFAULT_handshake_send ""
#define FIELD_N_handshake_send 3
#define FORMAT_handshake_send "s"
#define IS_PTR_handshake_send 1

struct config_t {
    unsigned fields;

    int port;
    char handshake_recv[HANDSHAKE_MAX];
    char handshake_send[HANDSHAKE_MAX];
};

int read_from(char *path, struct config_t *config, int else_use_default);

#endif // CONFIG_H
