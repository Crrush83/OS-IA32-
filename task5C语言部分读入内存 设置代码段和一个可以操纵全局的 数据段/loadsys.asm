         ;代码清单11-1
         ;文件名：c11_mbr.asm
         ;文件说明：硬盘主引导扇区代码 
         ;创建日期：2011-5-16 19:54
         ; hello-os                                                                                   
     sys_lba_start equ 100

                                  
;对不起 还是打算在实模式下
          ;设置堆栈段和栈指针 
          ;就是现在的cs可能不是0你知道吗！然后进入保护模式之后并不是0x7c00
         mov ax,cs      
         mov ss,ax
         mov sp,0x7c00                   ;用户程序放在内存的哪里应该在引导程序中专门用一个地方记录
										 ;位于磁盘的哪里专门用一个常量记录
         mov ax,[cs:phy_base+0x7c00]            ;计算用于加载用户程序的逻辑段地址 
         mov dx,[cs:phy_base+0x7c00+0x02]      ;phy_base是本文件中的一个标号 在哪里写有0x100000 是用户程序将要被加载到的绝对地址n这已经1M了
         mov bx,16  ;你这样讲 溢出了 懂吗？0x100000 / 0x10 = 0x10000 大于16位了这就
            ;而且说实模式他就限制你在1M之内
         div bx 
         
         mov cx,1 ;32个扇区啊
readsector:
         xor di,di ;目的地址偏移值
         mov si,sys_lba_start             ;程序在硬盘上的起始逻辑扇区号 
         add si,1
         sub si,cx
         ;扇区号 = 100 + 总扇区-已读扇区
   
         mov ds,ax                       ;令DS和ES指向该段以进行操作                                           ;而如今附加段和数据段都要以0x10000为基址
         mov es,ax   
         xor bx,bx                       ;加载到DS:0x0000处 ?
         call read_floppy_disk_0         ;call指令暗含着要执行别处的代码
         add ax,0x20 ;向后512B
         ;128kb的系统要读很多次
         loop readsector
         
         jmp loadgdt
        ;手动保护现场和恢复ip

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
    read:
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

loadgdt:
         ;设置堆栈段和栈指针 
         mov ax,cs      
         mov ss,ax
         mov sp,0x7c00
      
         mov ax,[cs:gdt_base+0x7c00]        ;低16位 
         mov dx,[cs:gdt_base+0x7c00+0x02]   ;高16位 
         mov bx,16        
         div bx            
         mov ds,ax                          ;令DS指向该段以进行操作
         mov bx,dx                          ;段内起始偏移地址 
      
         ;创建0#描述符，它是空描述符，这是处理器的要求
         mov dword [bx+0x00],0x00
         mov dword [bx+0x04],0x00  

         ;创建#1描述符，保护模式下的代码段描述符
		 ;偏移0-15 16-19 第一行是低地址 段内偏移0-001ff 1后面9个0 2^9 = 512字节
         mov dword [bx+0x08],0x7c0001ff     
         mov dword [bx+0x0c],0x00409800     

         ;创建#2描述符，保护模式下的数据段描述符（文本模式下的显示缓冲区） 
		 ;0xb8000 文本模式显存字符区
		 ;段内偏移 - 0ffff =2^16Byte = 2^8 * 256
         mov dword [bx+0x10],0x8000ffff     
         mov dword [bx+0x14],0x0040920b    ;4 9 2
         ;想弄一个保护模式下的sys         

         ;创建#3描述符，保护模式下的堆栈段描述符
		 ;
         mov dword [bx+0x18],0x00007a00
         mov dword [bx+0x1c],0x00409600
         
         ;加载c程序开始的位置 分配大小为128KB的空间作为code段
		 ;低
		 ;高 段选择子是4号
         mov dword [bx+0x20],0x0000ffff
         mov dword [bx+0x24],0x00419d10 ;9810改为9d10
		 
		 ;再加一个全局(32M)(4GB)可用内存
		 mov dword [bx+0x28],0x0000ffff ;28+4 = 2c 十六进制计算出错了哈
		 mov dword [bx+0x2c],0x00cf9200 ;c 9 2 只是段粒度是4KB G位为1
         
         ;创建#6描述符，保护模式下的代码段描述符
		 ;偏移0-15 16-19 第一行是低地址 段内偏移0-001ff 1后面9个0 2^9 = 512字节
         mov dword [bx+0x30],0x7c0001ff    
         mov dword [bx+0x34],0x00409000 ;只读的数据段啊
         
         ;#7 0x10000开始长达128KB的可读数据段
          ;初始化描述符表寄存器GDTR
         mov word [cs:gdt_size+0x7c00],55  ;描述符表的界限（总字节数减一）   
                                             
         lgdt [cs: gdt_size+0x7c00]
      
         in al,0x92                         ;南桥芯片内的端口 
         or al,0000_0010B
         out 0x92,al                        ;打开A20

         cli                                ;保护模式下中断机制尚未建立，应 
                                            ;禁止中断 
         mov eax,cr0
         or eax,1
         mov cr0,eax                        ;设置PE位
      
         ;以下进入保护模式... ...
         jmp dword 0x0008:flush             ;16位的描述符选择子：32位偏移
                                            ;清流水线并串行化处理器 
         [bits 32] 

    flush:
         mov cx,00000000000_10_000B         ;加载数据段选择子(0x10)
         mov ds,cx

         ;以下在屏幕上显示"Protect mode OK." 
         mov byte [0x00],'P'  
         mov byte [0x02],'r'
         mov byte [0x04],'o'
         mov byte [0x06],'t'
         mov byte [0x08],'e'
         mov byte [0x0a],'c'
         mov byte [0x0c],'t'
         mov byte [0x0e],' '
         mov byte [0x10],'O'
         mov byte [0x12],'K'

         ;以下用简单的示例来帮助阐述32位保护模式下的堆栈操作 
         mov cx,00000000000_11_000B         ;加载堆栈段选择子
         mov ss,cx
         mov esp,0x7c00
         
         mov ebp,esp                        ;保存堆栈指针 
         push byte '.'                      ;压入立即数（字节）
         
         sub ebp,4
         cmp ebp,esp                        ;判断压入立即数时，ESP是否减4 
         jnz ghalt                          
         pop eax
         mov [0x14],al                      ;显示句点 
         ;跳转到main 能吗？		 

         ;移动0x10000~0x2ffff 到 0x100000~11ffff
           ;ds换成全局数据段?虽然粒度是4kb 但那只是为计算偏移准备的吧
         ;偏移值20
         
         mov cx,0000000000101000B ;就这里不对 怎么回事？
         mov ds,cx
         mov esi,0x10000
         mov edi,0x100000
         mov ecx,0x100
         move:
         mov edx,[esi]
         mov [edi],edx
         inc esi
         inc edi
         loop move
         
         jmp dword 0x0020:0x00000000 ;描述符5 4号000，10 0，000
      
  ghalt:     
         hlt                                ;已经禁止中断，将不会被唤醒 

;-------------------------------------------------------------------------------
         phy_base: 
                          dd 0x10000
         gdt_size         dw 0
         gdt_base         dd 0x00007e00     ;GDT的物理地址 
                             
         times 510-($-$$) db 0
                          db 0x55,0xaa