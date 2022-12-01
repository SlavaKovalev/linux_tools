obj-m += syscall_hook_5.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make clean
clean:
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf *.o *.mod* Module.symvers modules.order
