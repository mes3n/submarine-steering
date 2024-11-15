# Submarine Mainframe

Sockets based communication with the submarine's GPIO pins.

## Setup

This project uses Pigpio for interfacing with the GPIO pins on the Raspberry Pi. Install with:
```bash
sudo apt install pigpio
```

If the Pigpio library is unavailable it is possible to compile the project
without an GPIO interface.

Most relevant settings can be configured through `main.conf`, the current available settings are:

| *key*          | *type*  | *default* |
| -------------- | ------- | --------- |
| port           | `int`   | 2300      |
| handshake_recv | `char*` | MayIDr1ve |
| handshake_send | `char*` | YesYouMay |

The syntax supports comments `#` but requires `=` between key value pairs. Configured as such:
```conf
key = value  # Whitespaces are optional
foo=bar#abc  # Also valid
```

## Compilation

The project uses a makefile for simpler compilation.

The code can be compiled normally with:
```bash
make
```

The code can also be compiled without Pigpio by running:
```bash
make no_gpio
```

A complete rebuild can be done with:
```bash
make all
```

## Execution

After compiling the code the project can simply be run with:
```bash
make run
```

This will start a server on `port` that makes a handshake with the new connection
using `handshake_recv` and `handshake_send`. If this succeeds the server starts using
the recieved data for steering the submarine.

## Forking

Most of the server code is pretty standard as there are only minimal changes to the
standard method of configuring a network socket in C.

So far the GPIO interface is also pretty simple altough it might be expanded upon along
with implementing support for motor control.

The configuration files are less straightforward. In order to simplify the process
of modifying fields, some macros have been utilized. Adding a new field requires the following changes:
```c
// src/config.h

struct Config {
    // ...
    int new_var;
}

#define DEFAULT_new_var 123  // A default value if none is provided from conf
#define FIELD_N_new_var 4  // A unique 0 <= n < 32 used for management
#define FORMAT_new_var "d"  // scanf formatting for reading from conf
#define IS_PTR_new_var 0  // Since the value should not be used as a (char *)
```

```c
// src/config.c
// ...
int scan_line(char *line, struct Config *config) {
    __eval__(__map__(__set_var__, /*...*/, new_var));
    return 0;
}
// ...
     // Only needed if the new_var should be zero-initialized
    __eval__(__map__(__zero_var__, /*...*/, new_var);
    // ...
    if (use_default) {
        __eval__(
            __map__(__default_var__, /*...*/, new_var));
    }
// ...
```

______
_Alternative Names:_ submarine-core*, submarine-mainframe*, submarine-central, submarine-server
