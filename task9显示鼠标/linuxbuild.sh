cd /root/OSwork
nasm asmfunc.asm -f elf -o asmfunc.bin
nasm hankaku.asm -f elf -o hankaku.bin
gcc -Wall -m32 -g -c  os.c -o os.bin
gcc -Wall -m32 -g -c  osfunc.c -o osfunc.bin
gcc -Wall -m32 -g -c  info.c -o info.bin
ld -m elf_i386 --oformat binary asmfunc.bin os.bin osfunc.bin info.bin hankaku.bin -o sys.bin -T linuxlink.ld
exit
