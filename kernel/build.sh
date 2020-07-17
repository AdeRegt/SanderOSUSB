export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

nasm -felf32 stub/i386/grub/boot.asm -o boot.o || exit
nasm -felf32 hal/i386/isr.asm -o isr.o || exit
nasm -felf32 hal/i386/video.asm -o videoasm.o || exit
as -c hal/i386/paging.asm --32 -o paging2.o || exit

gcc -c kernel.c -m32 -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/io_ports.c -m32 -o io_ports.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/interrupts.c -m32 -o interrupts.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/paging.c -m32 -o paging.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/multitasking.c -m32 -o multitasking.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
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
gcc -c fs/iso9660.c -m32 -o iso9660.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/mbr.c -m32 -o mbr.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/ext.c -m32 -o ext.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/fat.c -m32 -o fat.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c exec/elf.c -m32 -o elf.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/math.c -m32 -o math.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/uhci.c -m32 -o uhci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ehci.c -m32 -o ehci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/xhci_hid.c -m32 -o xhci_hid.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/RTL8169.c -m32 -o RTL8169.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ethernet.c -m32 -o ethernet.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c exec/program.c -m32 -o program.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -T linker.ld -o myos.bin -m32 -ffreestanding -O2 -nostdlib boot.o kernel.o io_ports.o ehci.o paging.o paging2.o multitasking.o interrupts.o com_port.o ide.o pci.o memory.o timer.o video.o videoasm.o isr.o ps2.o device.o iso9660.o elf.o vbox.o xhci.o acpi.o ahci.o mbr.o ext.o fat.o math.o uhci.o xhci_hid.o RTL8169.o ethernet.o || exit

rm *.o

cp myos.bin ../kernel.bin
ar rcs ../lib/libsos.a ../kernel.bin  || exit

cd ..

for i in programs/*.asm
do
	nasm $i -o programs/`basename $i .asm`.bin  || exit
done

nasm -felf32 programs/base.as -o programs/base.o
for i in programs/*.c
do
	gcc -c $i -o programs/`basename $i .c`.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra || exit
	gcc -T programs/proglinker.ld  -m32 -O2 -ffreestanding -nostdlib programs/base.o programs/`basename $i .c`.o -o programs/`basename $i .c`.bin || exit
done

rm programs/*.o

rm cdrom.iso
mkdir mnt
mkdir mnt/prgs
cp programs/*.bin mnt/prgs
cp programs/fasm.bin mnt/prgs/fasm
cp test.asm mnt/test.asm
mkdir mnt/boot
mkdir mnt/boot/grub
cp kernel.bin mnt/kernel.bin
cp boot/grub/grub.cfg mnt/boot/grub/grub.cfg
grub-mkrescue -o cdrom.iso mnt  || exit
rm -rf mnt

