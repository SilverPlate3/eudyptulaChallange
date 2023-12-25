obj-m += main2.o

PWD := $(CURDIR)

all:
	-rmmod main2
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	echo "lsmod:"
	-lsmod | grep main2
	insmod main2.ko
	echo "----------"
	echo "lsmod:"
	lsmod | grep main2


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	-rmmod main2