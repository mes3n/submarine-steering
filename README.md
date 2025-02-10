# Submarine Mainframe

Sockets based communication with the submarine's GPIO pins.

## Setup

Most relevant settings can be configured through `main.conf`, the current
available settings are:

| *key*          | *type*  | *default* |
| -------------- | ------- | --------- |
| port           | `int`   | 2300      |
| handshake_recv | `char*` | MayIDr1ve |
| handshake_send | `char*` | YesYouMay |
| steer_x_pin    | `int`   | 2         |
| steer_y_pin    | `int`   | 3         |
| motor_ctrl_pin | `int`   | 4         |

The syntax supports comments `#` but requires `=` between key value pairs.
Configured as such:

```conf
key = value  # Whitespaces are optional
foo=bar#abc  # Also valid where 'foo=bar'
```

### Configuring ESC

Before the ESC is able to function normally it must be configured with the
maximum pulse width (max speed) and the minimum pulse width. This should be
done right as the ESC receives power.

```text
      |<-- 20ms -->|

MAX    2ms
      +---+        +---+        +---+        +---+        +---+
      |   |        |   |        |   |        |   |        |   |
      |   |        |   |        |   |        |   |        |   |
      |   |        |   |        |   |        |   |        |   |
      |   |        |   |        |   |        |   |        |   |
   ---+   +--------+   +--------+   +--------+   +--------+   +--

MIN   1ms
      +-+          +-+          +-+          +-+          +-+
      | |          | |          | |          | |          | |
      | |          | |          | |          | |          | |
      | |          | |          | |          | |          | |
      | |          | |          | |          | |          | |
   ---+ +----------+ +----------+ +----------+ +----------+ +----
```

> The graph above represent the pulses for setting the ESC maximum and
> its minimum. Note that the pulse width is 20ms for both.

This can be achieved using the `libgpiod-tools` and the following commands.
The first command should be running when the ESC is plugged into its power
source. The pin `P8_10` should be set according to the system configuration.
Wait some time before stopping the commands (`^C`) and starting the next one.

```bash
gpioset -t 2ms,18ms P8_10=1  # Wait some time after powering ESC (ca 5s)
gpioset -t 1ms,19ms P8_10=1  # Wait some time (ca 5s)
```

### libgpiod

The current implementation is mostly tailored towards the `libgpiod` library.
This option must be enabled during compilation and by installing `libgpiod` to
the target system.

### pigpio

> pigpio is primarily not supported

The application can be built using pigpio for interfacing with the GPIO pins on
the target. Install pigpio to the Raspberry Pi using `apt`.

```bash
sudo apt install pigpio
```

## Compilation

The project uses a makefile for simpler compilation.

The code can be compiled normally with `make`. However, this will yield a binary
without any support for interfacing with GPIO pins. This means only the server
side will be enabled.

```bash
make
```

Other options can be configured using `CONFIG_*` variables set either in the
shell or using a `.config` file.

The code can be compiled with *pigpio* by setting `CONFIG_BUILD_PIGPIO`. Note
that this requires `pigpio` headers to be installed.

```bash
echo CONFIG_BUILD_PIGPIO=y > .config
make
```

It is also possible to build using *libgpiod* by setting
`CONFIG_BUILD_LIBGPIOD`. Note that this requires `libgpiod` headers to be
installed.

```bash
echo CONFIG_BUILD_LIBGPIOD=y > .config
make
```

A complete rebuild can be also be done, this needs to be run when switching
between different configuration options.

```bash
make all
```

## Running

Starting the steering server can be done by running the built binary. Some
runtime options are provided and can be seen using the `-h` flag. Also note
that the configuration file can be determined using `-c <path>`.

```bash
./bin/steering
```

This will start a server on `port` that makes a handshake with the new
connection using `handshake_recv` and `handshake_send`. If this succeeds the
server starts using the received data for steering the submarine.

## Forking

The configuration files are a somewhat convoluted mess. In order to simplify the
process of modifying fields, some macros have been utilized. Adding a new field
will require defining some variables.

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
