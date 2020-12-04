KERNEL_PATH = /lib/modules/`uname -r`/build

obj-m += kprobe_test.o

all:
	make -C $(KERNEL_PATH) SUBDIRS=$(PWD) modules
	make clean
clean:
	rm -f *.o *.mod* Module.symvers modules.order
