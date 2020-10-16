gcc -I ../../include sedit.c -o ./sedit

fasm startup.asm startup.o							|| exit
gcc -I ../../include -c sedit.c -m32 -o sedit.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra -Wno-unused-parameter	|| exit
gcc -T linker.ld -o ./sedit -m32 -ffreestanding -O2 -nostdlib startup.o sedit.o || exit

cp ./sedit ../sedit.bin