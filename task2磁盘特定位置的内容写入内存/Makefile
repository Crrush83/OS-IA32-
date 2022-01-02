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
	#跳过输出文件的99个扇区（意思是写入逻辑上的第100个）
	$(DD) if=userpro.bin of=helloos.img bs=512 skip=0 seek=99
	
#将bochs需要的img文件放到bochs配置文件中规定的地方	
copyimg:img
	copy helloos.img ..\
#bochs运行在子任务文件夹中 默认读取运行路径下的配置文件
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
