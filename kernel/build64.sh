#! /bin/bash
# compile stub
echo "Assambling ASM files"
nasm -felf64 hal/x64/isr.asm -o isr.o || exit

echo "Compiling C files"
gcc stub/x64/uefi/uefi.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I /usr/include/efi/x86_64  -std=gnu99 -ffreestanding -O2 -Wall -Wextra -DEFI_FUNCTION_WRAPPER -o uefi.o || exit
gcc -c kernel.c -m64 -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/x64/io_ports.c -m64 -o io_ports.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c hal/x64/interrupts.c -m64 -o interrupts.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/com_port.c -m64 -o com_port.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ide.c -m64 -o ide.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/pci.c -m64 -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ps2.c -m64 -o ps2.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/memory.c -m64 -o memory.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/timer.c -m64 -o timer.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/video.c -m64 -o video.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/device.c -m64 -o device.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/xhci.c -m64 -o xhci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/vbox.c -m64 -o vbox.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/acpi.c -m64 -o acpi.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/AHCI.c -m64 -o ahci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/iso9660.c -m64 -o iso9660.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/mbr.c -m64 -o mbr.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/ext.c -m64 -o ext.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/fat.c -m64 -o fat.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c fs/sfs.c -m64 -o sfs.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c exec/elf.c -m64 -o elf.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/math.c -m64 -o math.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/uhci.c -m64 -o uhci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ehci.c -m64 -o ehci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/usb_hid.c -m64 -o xhci_hid.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/usb_stick.c -m64 -o ehci_stick.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/usb.c -m64 -o usb.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/RTL8169.c -m64 -o RTL8169.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c dev/ethernet.c -m64 -o ethernet.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
gcc -c exec/program.c -m64 -o program.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit

echo "Linking O files"
ld uefi.o kernel.o io_ports.o interrupts.o isr.o usb.o ehci.o ehci_stick.o com_port.o ide.o sfs.o pci.o memory.o timer.o video.o ps2.o device.o iso9660.o elf.o vbox.o xhci.o acpi.o ahci.o mbr.o ext.o fat.o math.o uhci.o xhci_hid.o RTL8169.o ethernet.o  /usr/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds -shared -Bsymbolic -L /usr/lib -l:libgnuefi.a -l:libefi.a -o sanderosusbefi.so || exit

echo "Adapting Executable"
objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-x86_64 sanderosusbefi.so sanderosusb.efi || exit

echo "Cleanup"
rm sanderosusbefi.so 
rm *.o
cp sanderosusb.efi ../sanderosusbefi.efi