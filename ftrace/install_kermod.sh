#!/bin/bash

kermod=ftrace_test
rm -rf ftrace_test.ko
echo qwerty | sudo -S rmmod "$kermod"
make
if [ -f "$kermod.ko" ]; then
	echo qwerty | sudo -S insmod ftrace_test.ko
else
	echo "file $kermod.ko doesn't exist"
fi
