obj-m += rootkit.o
rootkit-objs := main.o misc.o hidefs.o syscalls.o procfs.o hide.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean