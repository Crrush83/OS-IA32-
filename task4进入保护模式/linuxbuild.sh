cd /root/OSwork
nasm asmfunc.asm -f elf -o asmfunc.bin
gcc -Wall -m32 -g -c  os.c -o os.bin
ld -m elf_i386 --oformat binary asmfunc.bin os.bin -o sys.bin -T linuxlink.ld
dd if=/dev/zero of=helloos.img bs=512 count=2880
dd if=loadsys.bin of=helloos.img bs=512 skip=0 seek=0
dd if=sys.bin of=helloos.img bs=512 skip=0 seek=99 conv=noerror
exit
