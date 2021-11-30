#!/bin/bash
fasm stub.asm stub.o
cp ../../../DragonCompiler/Makefile ./makefilebackup
cp ./SanderOSUSBMakefile ../../../DragonCompiler/Makefile
cp ./linker.ld ../../../DragonCompiler/linker.ld
cd ../../../DragonCompiler
make clean
make
rm ./linker.ld
cd ../SanderOSUSB/programs/dragoncompiler
cp ./makefilebackup ../../../DragonCompiler/Makefile