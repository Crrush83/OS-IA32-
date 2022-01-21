cd /root/OSwork
nasm io.asm -f elf -o asmio.bin
nasm dsctb.asm -f elf -o asmdsctb.bin
nasm hankaku.asm -f elf -o hankaku.bin
nasm time.asm -f elf -o asmtime.bin
nasm interupt.asm -f elf -o asminterupt.bin

gcc -Wall -m32 -g -c  os.c -o os.bin
gcc -Wall -m32 -g -c  graphic.c -o graphic.bin
gcc -Wall -m32 -g -c  dsctb.c -o dsctb.bin
gcc -Wall -m32 -g -c  time.c -o time.bin


ld -m elf_i386 --oformat binary asmio.bin os.bin graphic.bin dsctb.bin asmdsctb.bin hankaku.bin asmtime.bin time.bin asminterupt.bin  -o sys.bin -T linuxlink.ld
exit
