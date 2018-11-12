nasm -felf32 boot.asm -o boot.o
gcc -c kernel.c -m32 -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
gcc -T linker.ld -o myos.bin -m32 -ffreestanding -O2 -nostdlib boot.o kernel.o
if grub-file --is-x86-multiboot myos.bin; then
  cp myos.bin ../kernel.bin
  qemu-system-i386 -kernel myos.bin
else
  echo the file is not multiboot
fi
