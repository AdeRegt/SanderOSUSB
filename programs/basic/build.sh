fasm startup.asm startup.o							|| exit
gcc -I ../../include -c basic.c -m32 -o basic.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra -Wno-unused-parameter	|| exit
gcc -T linker.ld -o ./basic -m32 -ffreestanding -O2 -nostdlib startup.o basic.o ../../lib/user32.o ../../lib/modern.o || exit
cp ./basic ../basic.bin