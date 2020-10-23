# SanderOSUSB

Thank you for showing interest in SanderOSUSB!
SanderOSUSB is a 32bit single tasking monolythic kernel that is supposed to run from USB or cd-rom.

## Compiling
For compiling you need the following programs:
- gcc
- nasm
- xorriso

``` sudo apt install gcc nasm xorriso ```

And then run the file **/kernel/build.sh**

## Runing the OS
- install qemu

```sudo apt install qemu qemu-utils qemu-kvm```

- go to the repo directory
- run:

```qemu-system-x86_64 -cdrom ./cdrom.iso```

## Suported Devices
Currently the following devices are supported:
* IDE - ATA
* IDE - ATAPI
* AHCI - ATA
* AHCI - ATAPI
* PS/2 keyboard
* PS/2 mouse
* PIC
* PCI
* ACPI
* USB - HID - Keyboard
* USB - Mass Storage Device
* EHCI - Basic support
* RTL8169
* Soundblaster 16

The following devices are on the list to get implemented:

The following filesystems are supported:
* ISO 9660 (R)
* MBR (R)
* EFI-MBR (R)
* EXT2 (very basic) (R)
* FAT 32 (R)
* SFS (RW)

The following filesystems will be supported:
* FAT 12

the following executable formats are supported:
* 32-ELF

the following executable formats are on the list to get implemented:
* 32-MZPE
* 32-MZCOFF
* Marble
* Mike-BASIC

To build the kernel, go to the kernel folder and run build.sh

## Directories
* boot: Grub bin folder
* kernel: Kernel source code
* lib: kernel static library to which programs attach to
* programs: example programs
