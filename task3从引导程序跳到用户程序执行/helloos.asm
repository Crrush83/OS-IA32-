; hello-os                                                                                   
     app_lba_start equ 100

	;没有磁盘设备 我读软盘行不？
    SECTION mbr align=16 vstart=0x7c00                                     

         ;设置堆栈段和栈指针 
         mov ax,0 		 
		; mov cs,ax
         mov ss,ax
         mov sp,ax
                                         ;用户程序放在内存的哪里应该在引导程序中专门用一个地方记录
										 ;位于磁盘的哪里专门用一个常量记录
         mov ax,[cs:phy_base]            ;计算用于加载用户程序的逻辑段地址 
         mov dx,[cs:phy_base+0x02]      ;phy_base是本文件中的一个标号 在哪里写有0x10000 是用户程序将要被加载到的绝对地址
         mov bx,16        
         div bx            
         mov ds,ax                       ;令DS和ES指向该段以进行操作
                                           ;而如今附加段和数据段都要以0x10000为基址
         mov es,ax      
                                        ;想要0x10000上有那部分程序 首先要读进来 诸个寄存器                                                                                             
                                         ;以下读取程序的起始部分 
         xor di,di
         mov si,app_lba_start            ;程序在硬盘上的起始逻辑扇区号 
         xor bx,bx                       ;加载到DS:0x0000处 ?
         call read_floppy_disk_0         ;call指令暗含着要执行别处的代码
										 ;手动保护现场和恢复ip
	     
		 ;此刻已经从调用过程返回 根据读取的第一个扇区 判断程序的信息
		 mov dx,[2]    ;mov指令使用的默认的段寄存器是ds 已经被我们置为用户程序在内存中的起始地址
		 mov ax,[0]    ;用户程序前四个字节应该是整个程序的大小 32位 也是被除数的大小
		 mov bx,512
		 div bx
		 cmp dx,0     ;余数是0 则代表ax中的商就是要读取的扇区数 接着执行-1 因为刚刚已经读取了第一个扇区
		 jnz @1       ;如果余数不为0 说明实际的扇区数应该是结果+1 但是已经读取了一个扇区 所以剩余要读取的扇区数是结果本身 跳过-1的语句即可
		 dec ax       

@1:
         cmp ax,0
         jz direct
         ;ax 剩余的要读取的扇区数 如果不为0 就要继续把它们读进来
         ;否则的话 继续读取剩余的扇区内容
         ;目前的ds是用户程序起始位置的段地址
         ;但读取剩余扇区的过程中 也要用到不断修改ds
         push ds
         mov cx,ax
;读取剩余的部分
@2:
         mov ax,ds
         add ax,0x20 ;512/16 段寄存器增加的地址
         mov ds,ax
         xor bx,bx   ;bx作为偏移量 也是读取软盘的BIOS程序规定的内存偏移量
         inc si      ;si是逻辑扇区号 第一次读取的时候si是用户程序在磁盘中的位置
         call read_floppy_disk_0
         loop @2     ;loop 暗含cx-1 并检查是否为0 然后跳转到指定标号的地方

         pop ds      ;读取磁盘内容结束 恢复ds的意义：用户程序在内存中的段偏移
         
;剩余部分读取结束 跳转到用户程序执行
direct:
         ;用户程序的前四个字节：程序大小
         ;用户程序的5~10个字节：低地址段内偏移量 高地址段偏移
         ;没有被引导程序加载的时候 段偏移当然是不知道的
         ;段偏移定在哪里 是引导程序决定的 用户程序只能空出来等待填充
         mov dx,[0x08]
         mov ax,[0x06]
         call calc_segment_base
         mov [0x06],ax  ;段偏移只有16位 存在低地址
         
         ;处理段表重定位
         ;在程序大小、入口地址位置之后的0x0a位置 存放了段表项的数量
         ;此前保护了dx 所以dx仍然是用户程序在内存中的起始位置
         mov cx,[0x0a]
         mov bx,0x0c
realloc:
         mov dx,[bx+0x02]
         mov ax,[bx]
         call calc_segment_base
         mov [bx],ax
         add bx,4
         loop realloc
         
         jmp far [0x04] ;jmp far指令默认访问6位数据 低位16位偏移 高位段基址
         ;更改cs ip 在用户程序中修改了栈段 如果要返回得记录引导程序的ss sp
         ;取指就是取CS IP啊 但是有一些东西只能存在栈里 但是栈在哪里呢 是由用户程序自己设置的
         
;------------------still section mbr---------------------------------
read_floppy_disk_0:
         ;输入：di:si=起始逻辑扇区号         
         ;      ds:bx=目标缓冲区地址
		 push ax
         push bx
         push cx
         push dx
         ;确定柱面 -> CH 0~79
		 ;确定磁头 -> DH 0 1
		 ;确定扇区 -> CL 1~18
		 xor dx,dx ;高十六位置0 因为软盘一共只有2880个扇区
		 mov ax,si
		 sub ax,1  ;1~2880 1~36 * 80
		           ;0~2879
		 mov cl,36
		 div cl
		           ;余数0 eg:36 柱面数1 余数换算为0面1扇区
	     mov ch,al
		 mov cl,8  ;右移的量需要存放在cl中
		 shr ax,cl ;ah部分（余数）是新的除数
		 mov cl,18
		 div cl    ;商 = 磁头号 0/1 余数+1 = 扇区号
		 mov dh,al
		 mov cl,ah
		 add cl,1   ;read 开始 es做内存段寄存器
    read：
         mov ah,0x02
		 mov al,1
		 mov dl,0x00
		 INT 0x13
		 JC err ;出错了 重读一次 也不要重读太多 
		 ;hlt
		 err:
         pop dx
         pop cx
         pop bx
         pop ax
         
         ret
;--------------------------------------------------------
calc_segment_base:
         push dx
         
         add ax,[cs:phy_base]   ;用户程序中分了段 但段偏移处也只能写段相对于用户程序起始的偏移量 
         add dx,[cs:phy_base+0x02]  ;在这里 我们要计算出来绝对内存偏移
                                    ;然后转换为段寄存器内容
         shr ax,4
         ror dx,4   ;实模式下只有20位地址可用 所以只有dx的高四位是有效的
         and dx,1111_0000_0000_0000B
         or ax,dx   ;这一步很巧妙 dx的低四位和ax的高16位合并
         
         pop dx
         
         ret
;--------------------------------------------------------         

;--------------------------------------------------------		 
phy_base: 
  dd 0x10000
  RESB  0x1fe - ($ - $$)  ; 填写0x00，直到0x001fe
  DB    0x55, 0xaa
