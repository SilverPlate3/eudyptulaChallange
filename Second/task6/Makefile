obj-m += main.o
ccflags-y += -DDEBUG

PWD := $(CURDIR)

all:
	-rmmod main
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
	-insmod main.ko
	-lsmod | grep main

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	-rmmod main
	-lsmod | grep main