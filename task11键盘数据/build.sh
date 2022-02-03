rm -rf bin
mkdir bin
cd bin
nasm -f bin ../loadsys.asm -o loadsys.bin
nasm ../io.asm -f elf -o asmio.bin
nasm ../hankaku.asm -f elf -o hankaku.bin
nasm ../time.asm -f elf -o asmtime.bin
nasm ../interupt.asm -f elf -o asminterupt.bin
gcc -Wall -m32 -g -c  -I ../include -I ../libc/include ../libc/stdio/stdio.c -o stdio.bin
gcc -Wall -m32 -g -c  -I ../include -I ../libc/include ../os.c -o os.bin
gcc -Wall -m32 -g -c  -I ../include -I ../libc/include  ../graphic.c -o graphic.bin
gcc -Wall -m32 -g -c  -I ../include -I ../libc/include ../time.c -o time.bin
gcc -Wall -m32 -g -c  -I ../include -I ../libc/include ../interupt.c -o interupt.bin
gcc -Wall -m32 -g -c  -I ../include -I ../libc/include ../fifo.c -o fifo.bin
gcc -Wall -m32 -g -c  -I ../include -I ../libc/include ../keyboardmouse.c -o keyboardmouse.bin
ld -m elf_i386 --oformat binary asmio.bin os.bin graphic.bin hankaku.bin asmtime.bin time.bin asminterupt.bin  interupt.bin fifo.bin keyboardmouse.bin stdio.bin -o sys.bin -T '../linkscript.ld'
#make img

