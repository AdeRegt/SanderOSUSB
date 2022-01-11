gcc -I ../../include -c chat.c -m32 -o chat.o  -m32  -std=gnu99 -ffreestanding -Wall -Wextra -Wno-unused-parameter	|| exit
gcc -T ./../../lib/linker.ld -o ./chat -m32 -ffreestanding -O2 -nostdlib chat.o ./../../lib/user32.o ./../../lib/modern.o ./../../lib/stub.o || exit
cp ./chat ../chat.bin