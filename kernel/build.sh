nasm -felf32 stub/i386/grub/boot.asm -o boot.o || exit
nasm -felf32 hal/i386/isr.asm -o isr.o || exit

gcc -c kernel.c -m32 -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/io_ports.c -m32 -o io_ports.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/interrupts.c -m32 -o interrupts.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/com_port.c -m32 -o com_port.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ide.c -m32 -o ide.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/pci.c -m32 -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ps2.c -m32 -o ps2.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/memory.c -m32 -o memory.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/timer.c -m32 -o timer.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/video.c -m32 -o video.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/device.c -m32 -o device.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/iso9660.c -m32 -o iso9660.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit

gcc -T linker.ld -o myos.bin -m32 -ffreestanding -O2 -nostdlib boot.o kernel.o io_ports.o interrupts.o com_port.o ide.o pci.o memory.o timer.o video.o isr.o ps2.o device.o iso9660.o || exit

cp myos.bin ../kernel.bin
cd ..
nasm programs/*.asm
rm cdrom.iso
cd ..
grub-mkrescue -o SanderOSUSB/cdrom.iso SanderOSUSB
cd SanderOSUSB
