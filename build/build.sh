nasm -felf32 boot.asm -o boot.o
gcc -c kernel.c -m32 -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
gcc -T linker.ld -o myos.bin -m32 -ffreestanding -O2 -nostdlib boot.o kernel.o
cp myos.bin ../kernel.bin
cd ..
rm cdrom.iso
cd ..
grub-mkrescue -o SanderOSUSB/cdrom.iso SanderOSUSB
cd SanderOSUSB
qemu-system-i386 -kernel kernel.bin
