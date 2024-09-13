#include <stdio.h>
#include <string.h>

#include "config.h"

#define CHAR_PER_LINE 128

#define __eval__(...) __eval1024__(__VA_ARGS__)
#define __eval1024__(...) __eval512__(__eval512__(__VA_ARGS__))
#define __eval512__(...) __eval256__(__eval256__(__VA_ARGS__))
#define __eval256__(...) __eval128__(__eval128__(__VA_ARGS__))
#define __eval128__(...) __eval64__(__eval64__(__VA_ARGS__))
#define __eval64__(...) __eval32__(__eval32__(__VA_ARGS__))
#define __eval32__(...) __eval16__(__eval16__(__VA_ARGS__))
#define __eval16__(...) __eval8__(__eval8__(__VA_ARGS__))
#define __eval8__(...) __eval4__(__eval4__(__VA_ARGS__))
#define __eval4__(...) __eval2__(__eval2__(__VA_ARGS__))
#define __eval2__(...) __eval1__(__eval1__(__VA_ARGS__))
#define __eval1__(...) __VA_ARGS__

#define _empty()
#define __defer__(m) m _empty()
#define __defer2__(m) m _empty _empty()()

#define __cat__(a, b) a##b

#define _first(a, ...) a
#define _second(a, b, ...) b
#define _is_probe(...) _second(__VA_ARGS__, 0)
#define _probe() ~, 1

#define _not(x) _is_probe(__cat__(_not_, x))
#define _not_0 _probe()

#define __bool__(x) _not(_not(x))

#define __if_else__(cond) _if_else(__bool__(cond))
#define _if_else(cond) __cat__(_if_, cond)

#define _if_1(...) __VA_ARGS__ _if_1_else
#define _if_0(...) _if_0_else
#define _if_1_else(...)
#define _if_0_else(...) __VA_ARGS__

#define _end_of_args() 0
#define __has_args__(...) __bool__(_first(_end_of_args __VA_ARGS__)())

#define __get_args__(...) __VA_ARGS__

#define _map() __map__
#define __map__(m, first, ...)                                                 \
    m(first) __if_else__(__has_args__(__VA_ARGS__))(                           \
        __defer2__(_map)()(m, __VA_ARGS__))()

#define __str__(s) #s
#define __set_var__(var)                                                       \
    {                                                                          \
        if (sscanf(line, " " __str__(var) " = %" FORMAT_##var " ",             \
                   __if_else__(IS_PTR_##var)()(&)config->var) == 1) {          \
            config->fields |= 0x1 << FIELD_N_##var;                            \
            return 1;                                                          \
        }                                                                      \
    }

int scan_line(char *line, struct Config *config) {
    __eval__(__map__(__set_var__, port, handshake_recv, handshake_send));

    return 0;
}

void remove_comment(char *line) {
    char *c = memchr(line, '#', CHAR_PER_LINE);
    if (c != NULL)
        memset(c, 0, CHAR_PER_LINE - (c - line));
}

#define __is_custom__(var) (config->fields & (0x1 << FIELD_N_##var))
#define __default_var__(var)                                                   \
    if (!__is_custom__(var)) {                                                 \
        __if_else__(IS_PTR_##var)(                                             \
            strncpy(config->var, DEFAULT_##var, sizeof(config->var) - 1))(     \
            config->var = DEFAULT_##var);                                      \
    }

#define __zero_var__(var) memset(config->var, 0, sizeof(config->var));

int read_from(char *path, struct Config *config, int else_use_default) {
    FILE *fp;
    if ((fp = fopen(path, "r")) == NULL) {
        return -1;
    }

    config->fields = 0x0;
    __eval__(__map__(__zero_var__, handshake_recv, handshake_send));

    char buf[CHAR_PER_LINE];
    while (fgets(buf, CHAR_PER_LINE, fp)) {
        remove_comment(buf);
        if (sscanf(buf, " %*s") == EOF)
            continue;
        if (scan_line(buf, config))
            continue;
        printf("err: %s", buf);
    }

    fclose(fp);

    if (else_use_default) {
        __eval__(
            __map__(__default_var__, port, handshake_recv, handshake_send));
    }

    return 0;
}
