nasm -felf32 stub/i386/grub/boot.asm -o boot.o || exit
nasm -felf32 hal/i386/isr.asm -o isr.o || exit
nasm -felf32 hal/i386/video.asm -o videoasm.o || exit

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
gcc -c dev/xhci.c -m32 -o xhci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/vbox.c -m32 -o vbox.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/acpi.c -m32 -o acpi.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/AHCI.c -m32 -o ahci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/e1000.c -m32 -o e1000.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/iso9660.c -m32 -o iso9660.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c exec/elf.c -m32 -o elf.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit

ld -n -T linker.ld -o myos.bin -O2 -nostdlib -m elf_i386 boot.o kernel.o io_ports.o interrupts.o com_port.o ide.o pci.o memory.o timer.o video.o videoasm.o isr.o ps2.o device.o iso9660.o elf.o vbox.o xhci.o acpi.o ahci.o e1000.o || exit

rm *.o

cp myos.bin ../kernel.bin
cd ..

for i in programs/*.inc
do
	nasm -f elf32 -O0 -w+orphan-labels $i -o programs/`basename $i .inc`.o || exit
done

for i in programs/*.asm
do
	nasm -f elf32 -O0 -w+orphan-labels $i -o programs/`basename $i .asm`.o || exit
	gcc -T programs/proglinker.ld -m32 -ffreestanding -O2 -nostdlib programs/base.o programs/`basename $i .asm`.o -o programs/`basename $i .asm`.bin || exit
done

for i in programs/*.c
do
	gcc -c $i -o programs/`basename $i .c`.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra  || exit
	gcc -T programs/proglinker.ld -m32 -ffreestanding -nostdlib programs/base.o programs/`basename $i .c`.o -o programs/`basename $i .c`.bin || exit
done

rm programs/*.o

rm cdrom.iso
mkdir mnt
mkdir mnt/prgs
cp programs/*.bin mnt/prgs
rm programs/*.bin
mkdir mnt/boot
mkdir mnt/boot/grub
cp kernel.bin mnt/kernel.bin
cp boot/grub/grub.cfg mnt/boot/grub/grub.cfg

# thanks to Johan for multiple support
if [ -e /usr/bin/grub-mkrescue ]
then
    grub-mkrescue -o cdrom.iso mnt --iso-level 3
elif [ -e /usr/bin/grub2-mkrescue ]
then
    grub2-mkrescue -o cdrom.iso mnt --iso-level 3
fi

rm -rf mnt
