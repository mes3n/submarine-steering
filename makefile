include .config

PACKAGE ?= steering
DESTDIR ?= usr

CC_FLAGS = -Wall -g -O2
LD_FLAGS = -Wall -g -O2 -pthread
ifdef CONFIG_BUILD_LIBGPIOD
	CC_FLAGS += -DBUILD_LIBGPIOD
	LD_FLAGS += -lgpiod
else ifdef CONFIG_BUILD_PIGPIO
	CC_FLAGS += -DBUILD_PIGPIO
	LD_FLAGS += -lpigpio
endif

CC ?= gcc
OBJ = $(shell find src -name '*.c' | sed -e 's/\.c/\.o/' -e 's/src/obj/')
TARGET = bin/$(PACKAGE)

$(TARGET): file_tree $(OBJ)
	$(CC) $(OBJ) -o $@ $(LD_FLAGS)

obj/%.o: src/%.c
	$(CC) -c $< -o $@ $(CC_FLAGS)

.config:
	echo '# CONFIG_BUILD_PIGPIO is not set' >> $@
	echo '# CONFIG_BUILD_LIBGPIOD is not set' >> $@

install: $(TARGET)
	install -d $(dir $(DESTDIR)/$<)
	install $< $(DESTDIR)/$<

uninstall:
	rm $(DESTDIR)/$(TARGET)

all: clean $(TARGET)
	@echo $(CONFIG_BUILD_LIBGPIO)

file_tree:
	mkdir -p bin $(shell find src -type d | sed 's/src/obj/')

clean:
	rm -rf bin obj

