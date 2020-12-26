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
* :question: unknown
* :x: not supported
* :white_check_mark: works

Devicename | VirtualBox | Qemu | Bochs | Real hardware
----------|---------- | ---- | ----- | -------------
IDE - ATA | :white_check_mark: | :white_check_mark: | :white_check_mark: | :question:
IDE - ATAPI | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
AHCI - ATA | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x:
AHCI - ATAPI | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x:
PS/2 keyboard | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
PS/2 mouse | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
PIC | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
PCI | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
ACPI | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
USB - HID - Keyboard | :white_check_mark: | :white_check_mark: | :white_check_mark: | :question:
USB - Mass Storage Device | :white_check_mark: | :white_check_mark: | :white_check_mark: | :x:
EHCI - Basic support | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
RTL8169 | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:
Soundblaster 16 | :white_check_mark: | :white_check_mark: | :white_check_mark: | :white_check_mark:

The following devices are on the list to get implemented:

The following filesystems are supported:

Filesystemname | Read | Write
-------------- | ---- | -----
ISO 9660 | :white_check_mark: | :x: 
MBR | :white_check_mark: | :x: 
EFI-MBR | :white_check_mark: | :x: 
EXT2 (very basic) | :white_check_mark: | :x: 
FAT 32 | :white_check_mark: | :x: 
SFS | :white_check_mark: | :white_check_mark: 
VirtualBoxFS | :x: | :x: 

The following filesystems will be supported:
* FAT 12

the following executable formats are supported:
* 32-ELF (relocation and executable type)
* raw binary at 0x2000
* BASfile (with basic.bin)

the following executable formats are on the list to get implemented:
* 32-MZPE
* 32-MZCOFF
* Marble

To build the kernel, go to the kernel folder and run build.sh

## Directories
* boot: Grub bin folder
* kernel: Kernel source code
* lib: kernel static library to which programs attach to
* programs: example programs
