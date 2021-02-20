#!/bin/bash

kermod=syscall_table_dump
rm -rf $kermod.ko
echo qwerty | sudo -S rmmod "$kermod"
make
if [ -f "$kermod.ko" ]; then
	echo qwerty | sudo -S $kermod.ko
else
	echo "file $kermod.ko doesn't exist"
fi
