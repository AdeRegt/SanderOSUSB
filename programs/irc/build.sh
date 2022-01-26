gcc -I ../../include -c irc.c -m32 -o irc.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra -Wno-unused-parameter	|| exit
gcc -T ./../../lib/linker.ld -o ./irc -m32 -ffreestanding -O2 -nostdlib irc.o ./../../lib/user32.o ./../../lib/modern.o ./../../lib/stub.o || exit
cp ./irc ../irc.bin