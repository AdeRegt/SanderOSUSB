#!/bin/bash                                                                                                     # tell what we are
fasm fasm.asm core.o								|| exit                                                     # compile fasm core
fasm startup.asm startup.o							|| exit                                                     # compile bootstrap
gcc -T linker.ld -o fasm -m32 -ffreestanding -O2 -nostdlib startup.o core.o ../../../../lib/user32.o || exit    # link bridge
rm *.o                                                                                                          # remove any objects