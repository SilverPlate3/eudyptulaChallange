obj-m += main3.o
ccflags-y += -DDEBUG

PWD := $(CURDIR)

all:
	-rmmod main3
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
	-insmod main3.ko
	-lsmod | grep main3

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	-rmmod main3
	-lsmod | grep main3