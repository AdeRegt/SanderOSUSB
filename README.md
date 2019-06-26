# SanderOSUSB

Thank you for showing interest in SanderOSUSB!
SanderOSUSB is a 32bit single tasking monolythic kernel that is supposed to run from USB or cd-rom.

Currently the following devices are supported:
* IDE - ATA
* IDE - ATAPI
* AHCI - ATA
* AHCI - ATAPI (only at real hardware)
* PS/2 keyboard
* PS/2 mouse (only at real hardware)
* PIC
* PCI
* ACPI

The following devices are on the list to get implemented:
* EHCI
* XHCI

The following filesystems are supported:
* ISO 9660

The following filesystems will be supported:
* FAT 32
* FAT 12
* EXT2

To build the kernel, go to the kernel folder and run build.sh
