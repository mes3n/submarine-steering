-include .config

PACKAGE ?= steering
DESTDIR ?= /usr

CC_FLAGS = -Wall -g -O2
LD_FLAGS = -Wall -g -pthread
ifdef ($(CONFIG_BUILD_PIGPIO))
	LD_FLAGS += -lpigpio
	CC_FLAGS += -DBUILD_PIGPIO
endif

CC = gcc
OBJ = $(shell find src -name '*.c' | sed -e 's/\.c/\.o/' -e 's/src/obj/')
TARGET = bin/$(PACKAGE)

$(TARGET): file_tree $(OBJ)
	$(CC) $(OBJ) -o $@ $(LD_FLAGS)

obj/%.o: src/%.c
	$(CC) -c $< -o $@ $(CC_FLAGS)

install: $(TARGET)
	install -d $(dir $(DESTDIR)/$(TARGET))
	install $(TARGET) $(DESTDIR)/$(TARGET)

uninstall:
	rm $(DESTDIR)/$(TARGET)

all: clean $(TARGET)

file_tree:
	mkdir -p bin $(shell find src -type d | sed 's/src/obj/')

clean:
	rm -rf bin obj

