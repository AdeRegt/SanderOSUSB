#!/bin/bash
fasm stub.asm stub.o
cp ../../../DragonCompiler/Makefile ./makefilebackup
cp ./SanderOSUSBMakefile ../../../DragonCompiler/Makefile
cd ../../../DragonCompiler
make clean
make
cd ../SanderOSUSB/programs/dragoncompiler
cp ./makefilebackup ../../../DragonCompiler/Makefile