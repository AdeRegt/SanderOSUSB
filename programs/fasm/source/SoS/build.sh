#!/bin/bash
fasm fasm.asm core.o								|| exit
fasm startup.asm startup.o							|| exit
fasm sos.inc sos.o							        || exit
gcc -I ../../../../include -c api.c -m32 -o api.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra -Wno-unused-parameter	|| exit
gcc -T linker.ld -o fasm -m32 -ffreestanding -O2 -nostdlib startup.o core.o sos.o api.o || exit
rm *.o