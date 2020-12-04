# linux_tools
copy_delete.c -- copies just files (non recursively) from source folder to destination folder and delete files from source folder

new_device
register and unregister miscellaneous device.
How to build linux kernel module:
```make```
how to load module:
```sudo insmod new_decvie.ko```
how to check result of loading:
```lsmod | grep new_device```
how to view log records:
```dmesg | grep new_device```
how to unload module:
```sudo rmmod new_device```

kprobes
kprobe_test.c is add kprobe to openat sys call
See https://github.com/torvalds/linux/blob/master/samples/kprobes
https://www.kernel.org/doc/html/latest/index.html#
https://www.kernel.org/doc/html/latest/trace/kprobes.html
https://elixir.bootlin.com/linux/v5.9.11/source/include/linux/kprobes.h
https://elixir.bootlin.com/linux/v5.9.11/source/include/linux/syscalls.h