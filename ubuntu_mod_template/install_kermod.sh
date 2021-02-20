#!/bin/bash

kermod=ubuntu_tmpl
rm -rf $kermod.ko
echo qwerty | sudo -S rmmod "$kermod"
make
if [ -f "$kermod.ko" ]; then
	echo qwerty | sudo -S insmod $kermod.ko
else
	echo "file $kermod.ko doesn't exist"
fi
