KERNEL_MODULE_NAME := mdl
obj-m += $(KERNEL_MODULE_NAME).o
$(KERNEL_MODULE_NAME)-y += device.o
$(KERNEL_MODULE_NAME)-y += module.o
$(KERNEL_MODULE_NAME)-y += syscall_hook_5.o
$(KERNEL_MODULE_NAME)-y += transport.o
EXTRA_CFLAGS=-I$(PWD)
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	make clean
clean:
	#make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf *.o *.mod* Module.symvers modules.order
