KERNEL_PATH = /lib/modules/`uname -r`/build

obj-m += trace_event_sample.o

all:
	make -C $(KERNEL_PATH) SUBDIRS=$(PWD) modules
	make clean
clean:
	rm -f *.o *.mod* Module.symvers modules.order
