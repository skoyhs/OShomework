#!/bin/bash
nasm -f bin mbr.asm -o mbr.bin
nasm -f bin bootloader.asm -o bootloader.bin
qemu-img create hd1.img 10m
dd if=mbr.bin of=hd1.img bs=512 count=1 seek=0 conv=notrunc
dd if=bootloader.bin of=hd1.img bs=512 count=5 seek=1 conv=notrunc
