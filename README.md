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
https://lwn.net/Articles/132196/

ftrace
list of functions that available to trace with ftrace:
/sys/kernel/debug/tracing/available_filter_functions
See:
https://lwn.net/Articles/370423/
https://www.kernel.org/doc/html/latest/trace/ftrace-uses.html
https://www.apriorit.com/dev-blog/544-hooking-linux-functions-1
https://www.apriorit.com/dev-blog/546-hooking-linux-functions-2
https://www.kernel.org/doc/Documentation/trace/

tracing
https://lwn.net/Articles/379903/
https://github.com/torvalds/linux/blob/master/samples/trace_events/trace-events-sample.h
https://lwn.net/Articles/813350/

