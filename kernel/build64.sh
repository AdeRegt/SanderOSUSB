echo "64-bit kernel building"

x86_64-w64-mingw32-gcc -ffreestanding -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol -c -o uefi.o stub/uefi/uefi.c || exit

# KERNEL BUILD STANDARD STUFF
x86_64-w64-mingw32-gcc -DIS64 -c kernel.c -m64 -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c hal/i386/io_ports.c -m64 -o io_ports.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c hal/i386/interrupts.c -m64 -o interrupts.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c hal/i386/paging.c -m64 -o paging.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c hal/i386/multitasking.c -m64 -o multitasking.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/com_port.c -m64 -o com_port.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/ide.c -m64 -o ide.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/pci.c -m64 -o pci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/ps2.c -m64 -o ps2.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/memory.c -m64 -o memory.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/timer.c -m64 -o timer.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/video.c -m64 -o video.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/device.c -m64 -o device.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/xhci.c -m64 -o xhci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/vbox.c -m64 -o vbox.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/acpi.c -m64 -o acpi.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/AHCI.c -m64 -o ahci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c fs/iso9660.c -m64 -o iso9660.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c fs/mbr.c -m64 -o mbr.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c fs/ext.c -m64 -o ext.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c fs/fat.c -m64 -o fat.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c exec/elf.c -m64 -o elf.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/math.c -m64 -o math.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/uhci.c -m64 -o uhci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/ehci.c -m64 -o ehci.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/usb_hid.c -m64 -o xhci_hid.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/usb_stick.c -m64 -o ehci_stick.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/usb.c -m64 -o usb.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/RTL8169.c -m64 -o RTL8169.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/soundblaster16.c -m64 -o soundblaster16.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c dev/ethernet.c -m64 -o ethernet.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit
x86_64-w64-mingw32-gcc -DIS64 -c exec/program.c -m64 -o program.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra || exit

# KERNEL LINK

x86_64-w64-mingw32-gcc -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o ../KERNEL64.EFI uefi.o kernel.o io_ports.o soundblaster16.o usb.o ehci.o ehci_stick.o paging.o multitasking.o interrupts.o com_port.o ide.o pci.o memory.o timer.o video.o ps2.o device.o iso9660.o elf.o vbox.o xhci.o acpi.o ahci.o mbr.o ext.o fat.o math.o uhci.o xhci_hid.o RTL8169.o ethernet.o -lgcc || exit # data.o 

rm *.o

dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img ../KERNEL64.EFI ::/EFI/BOOT

mkdir iso
cp fat.img iso
xorriso -as mkisofs -R -f -e fat.img -no-emul-boot -o ../cdrom.iso iso
rm fat.img
rm -r -f iso