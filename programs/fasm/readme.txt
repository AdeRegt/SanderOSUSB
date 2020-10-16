
The fasm.o is an object file in ELF format. To get the final executable for
your system, you need to use the appropriate tool to link it with the C
library available on your platform. With the GNU tools it is enough to use
this command:

  gcc fasm.o -o fasm
