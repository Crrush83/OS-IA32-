NASM = C:\Users\11619\AppData\Local\bin\NASM\nasm.exe
QEMU = C:\Program Files\qemu\qemu-system-i386.exe
BOCHSDBG = C:\Program Files\Bochs-2.7\bochsdbg.exe
BOCHS = C:\Program Files\Bochs-2.7\bochs.exe
emptyVHD = C:\Users\11619\Desktop\毕设-小OS\emptyVHD.vhd\emptyVHD.vhd
PATH = C:\Users\11619\Desktop\毕设-小OS
DD = ..\tools\dd.exe
QEMUx86 = C:\Program Files\qemu\qemu-system-x86_64w.exe
    
iplbin:
	$(NASM)   helloos.asm -o helloos.bin
sysbin:
  
img: iplbin sysbin
	$(DD) if=/dev/zero of=helloos.img bs=512 count=2880
	$(DD) if=helloos.bin of=helloos.img bs=512 skip=0 seek=0
	
copyimg:img
	copy helloos.img ..\

bochsconf :
	copy ..\bochsrc.bxrc .\
	
bochs:copyimg bochsconf
	$(BOCHSDBG)

clean:
	rm -rf helloos.bin

qemu: img
	$(QEMU) -fda helloos.img
x86qemu: img
	$(QEMUx86) -fda helloos.img
	
	

.PHONY:
	qemu
