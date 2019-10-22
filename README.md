# SanderOSUSB

Thank you for showing interest in SanderOSUSB!
SanderOSUSB is a 32bit single tasking monolythic kernel that is supposed to run from USB or cd-rom.

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

The following devices are on the list to get implemented:
* EHCI
* XHCI

The following filesystems are supported:
* ISO 9660
* MBR
* EFI-MBR
* EXT2 (very basic)

The following filesystems will be supported:
* FAT 32
* FAT 12

the following executable formats are supported:
* 32-ELF

the following executable formats are on the list to get implemented:
* 32-MZPE
* 32-MZCOFF
* Marble
* Mike-BASIC

To build the kernel, go to the kernel folder and run build.sh
