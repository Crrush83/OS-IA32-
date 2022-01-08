cd /root/OSwork
nasm asmfunc.asm -f elf -o asmfunc.bin
gcc -Wall -m32 -g -c  os.c -o os.bin
gcc -Wall -m32 -g -c  osfunc.c -o osfunc.bin
ld -m elf_i386 --oformat binary asmfunc.bin os.bin osfunc.bin -o sys.bin -T linuxlink.ld
exit
