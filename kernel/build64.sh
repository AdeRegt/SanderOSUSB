echo "64-bit kernel building"

x86_64-w64-mingw32-gcc -ffreestanding -I/usr/include/efi -I/usr/include/efi/x86_64 -I/usr/include/efi/protocol -c -o uefi.o stub/uefi/uefi.c || exit
x86_64-w64-mingw32-gcc -nostdlib -Wl,-dll -shared -Wl,--subsystem,10 -e efi_main -o ../KERNEL64.EFI uefi.o -lgcc || exit # data.o 

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