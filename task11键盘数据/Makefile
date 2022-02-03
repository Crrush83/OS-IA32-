PATH = C:\Users\11619\Desktop\毕设-小OS
MINGW = C:\MinGW\bin
NASM := C:\Users\11619\AppData\Local\bin\NASM\nasm.exe
# -masm=intel 
GCC = $(MINGW)\gcc
LD = $(MINGW)\ld
DD = ..\tools\dd.exe
emptyVHD = C:\Users\11619\Desktop\毕设-小OS\emptyVHD.vhd\emptyVHD.vhd
QEMU = C:\Program Files\qemu\qemu-system-i386.exe
BOCHSDBG = C:\Program Files\Bochs-2.7\bochsdbg.exe
BOCHS = C:\Program Files\Bochs-2.7\bochs.exe
QEMUx86 = C:\Program Files\qemu\qemu-system-x86_64w.exe
# 矛盾的点在于 mingw的gcc的目标文件反汇编后是i386pe
# nasm可以生成i386pe 也可以elf i386
# 但需要纯二进制文件 windows版本的ld不支持纯二进制 
CFLAGS	:= -Wall -m32 -g c
LINUXHOST = 117.50.172.197
SCP = C:\Windows\System32\OpenSSH\scp.exe

writeimg:
	$(DD) if=/dev/zero of=helloos.img bs=512 count=2880
	$(DD) if=bin/loadsys.bin of=helloos.img bs=512 skip=0 seek=0
	$(DD) if=bin/sys.bin of=helloos.img bs=512 skip=0 seek=99 conv=noerror	
	copy helloos.img ..\

bochs: writeimg
	copy ..\bochsrc.bxrc 
	$(BOCHSDBG)
qemu: writeimg
	$(QEMU) -fda helloos.img
x86qemu: 
	$(QEMUx86) -fda helloos.img

.PHONY:
	qemu
