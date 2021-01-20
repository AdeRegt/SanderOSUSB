#! /bin/bash
# compile stub
gcc stub/x64/uefi/uefi.c -c -fno-stack-protector -fpic -fshort-wchar -mno-red-zone -I /usr/include/efi -I /usr/include/efi/x86_64  -std=gnu99 -ffreestanding -O2 -Wall -Wextra -DEFI_FUNCTION_WRAPPER -o uefi.o
ld uefi.o /usr/lib/crt0-efi-x86_64.o -nostdlib -znocombreloc -T /usr/lib/elf_x86_64_efi.lds -shared -Bsymbolic -L /usr/lib -l:libgnuefi.a -l:libefi.a -o sanderosusbefi.so
objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym -j .rel -j .rela -j .reloc --target=efi-app-x86_64 sanderosusbefi.so sanderosusb.efi
rm sanderosusbefi.so 
rm *.o
cp sanderosusb.efi ../sanderosusbefi.efi