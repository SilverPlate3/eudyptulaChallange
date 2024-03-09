obj-m += main3.o

PWD := $(CURDIR)

all:
	-rmmod main3
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	echo "lsmod:"
	-lsmod | grep main3
	insmod main3.ko
	echo "----------"
	echo "lsmod:"
	lsmod | grep main3


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	-rmmod main3