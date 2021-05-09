export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

echo "Build kernel" 
nasm -felf32 stub/i386/grub/boot.asm -o boot.o || exit
nasm -felf32 hal/i386/isr.asm -o isr.o || exit
nasm -felf32 hal/i386/video.asm -o videoasm.o || exit
as -c hal/i386/paging.asm --32 -o paging2.o  || exit

gcc -c kernel.c -m32 -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/io_ports.c -m32 -o io_ports.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/interrupts.c -m32 -o interrupts.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/paging.c -m32 -o paging.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/i386/multitasking.c -m32 -o multitasking.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/cpuid.c -m32 -o cpuid.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/com_port.c -m32 -o com_port.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ide.c -m32 -o ide.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/pci.c -m32 -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ps2.c -m32 -o ps2.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/memory.c -m32 -o memory.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/timer.c -m32 -o timer.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra || exit
gcc -c dev/video.c -m32 -o video.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/device.c -m32 -o device.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/xhci.c -m32 -o xhci.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra || exit
gcc -c dev/vbox.c -m32 -o vbox.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/acpi.c -m32 -o acpi.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/AHCI.c -m32 -o ahci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/iso9660.c -m32 -o iso9660.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/mbr.c -m32 -o mbr.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/ext.c -m32 -o ext.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/fat.c -m32 -o fat.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/sfs.c -m32 -o sfs.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c exec/elf.c -m32 -o elf.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/math.c -m32 -o math.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/uhci.c -m32 -o uhci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ehci.c -m32 -o ehci.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra || exit
gcc -c dev/usb_hid.c -m32 -o xhci_hid.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/usb_stick.c -m32 -o ehci_stick.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra || exit
gcc -c dev/usb.c -m32 -o usb.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/RTL8169.c -m32 -o RTL8169.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/e1000.c -m32 -o e1000.o -std=gnu99 -ffreestanding -O0 -Wall -Wextra || exit
gcc -c dev/soundblaster16.c -m32 -o soundblaster16.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ethernet.c -m32 -o ethernet.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c exec/program.c -m32 -o program.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit

gcc -T linker.ld -o myos.bin -m32 -ffreestanding -O2 -nostdlib boot.o kernel.o io_ports.o cpuid.o soundblaster16.o usb.o e1000.o ehci.o ehci_stick.o paging.o paging2.o multitasking.o interrupts.o com_port.o ide.o sfs.o pci.o memory.o timer.o video.o videoasm.o isr.o ps2.o device.o iso9660.o elf.o vbox.o xhci.o acpi.o ahci.o mbr.o ext.o fat.o math.o uhci.o xhci_hid.o RTL8169.o ethernet.o || exit

rm *.o

cp myos.bin ../kernel.bin
ar rcs ../lib/libsos.a ../kernel.bin

cd ..

rm include/symbols.h
rm symbols
objdump --syms kernel.bin >> symbols
cd utils
gcc gensymbols.c -o ./gensymbols
./gensymbols
cd ..

echo "Build programs"
for i in programs/*.asm
do
	nasm $i -o programs/`basename $i .asm`.bin || exit
done

nasm -felf32 programs/base.as -o programs/base.o || exit
for i in programs/*.c
do
	gcc -I include -c $i -o programs/`basename $i .c`.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra || exit
	gcc -T programs/proglinker.ld  -m32 -O2 -ffreestanding -nostdlib programs/base.o programs/`basename $i .c`.o -o programs/`basename $i .c`.bin || exit
done

cd programs
for D in */;
do
	echo "Compiling subprogram $D "
	cd $D
    bash build.sh || exit
	cd ..
done
cd ..

rm programs/*.o

#
# check what to do as output format
# possible options:
# - do nothing
# - grub
# - pxe
if [ $# -eq 0 ]
	then
		exit
fi 

if [ "$1" = "--grub" ]
	then
		echo "Build ISO"
		rm cdrom.iso
		mkdir mnt
		mkdir mnt/prgs
		cp programs/*.bin mnt/prgs
		mkdir mnt/boot
		mkdir mnt/boot/grub
		cp kernel.bin mnt/kernel.bin
		cp boot/grub/grub.cfg mnt/boot/grub/grub.cfg
		grub-mkrescue -o cdrom.iso mnt
		rm -rf mnt
fi

if [ "$1" = "--pxe" ]
	then
		cd boot/pxe 
		~/Downloads/smlrc -seg32 cstub.c cstub.asm
		nasm -fbin pxe_entry_point.asm -o pxestub.bin 
		cat pxestub.bin ../../kernel.bin > pxeentry.bin
		cp pxeentry.bin ../../SanderOSUSB.0
		exit
fi