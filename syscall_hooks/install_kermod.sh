#!/bin/bash

kermod=syscall_hook
rm -rf syscall_hook.ko
echo qwerty | sudo -S rmmod "$kermod"
make
if [ -f "$kermod.ko" ]; then
	echo qwerty | sudo -S insmod syscall_hook.ko
else
	echo "file $kermod.ko doesn't exist"
fi
