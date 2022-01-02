; hello-os

  app_lba_start equ 100

	;没有磁盘设备 我读软盘行不？
	SECTION mbr align=16 vstart=0x7c00                                     

         ;设置堆栈段和栈指针 
         mov ax,0 		 
		; mov cs,ax
         mov ss,ax
         mov sp,ax
      
         mov ax,[cs:phy_base]            ;计算用于加载用户程序的逻辑段地址 
         mov dx,[cs:phy_base+0x02]
         mov bx,16        
         div bx            
         mov ds,ax                       ;令DS和ES指向该段以进行操作
         mov es,ax      
         ;以下读取程序的起始部分 
         xor di,di
         mov si,app_lba_start            ;程序在硬盘上的起始逻辑扇区号 
         xor bx,bx                       ;加载到DS:0x0000处 ?
         call read_floppy_disk_0
		 

 
;------------------still section mbr---------------------------------
read_floppy_disk_0:
                                         ;输入：di:si=起始逻辑扇区号
                                         ;      ds:bx=目标缓冲区地址
		 ;确定柱面 -> CH 0~79
		 ;确定磁头 -> DH 0 1
		 ;确定扇区 -> CL 1~18
		 xor dx,dx ;高十六位置0 因为软盘一共只有2880个扇区
		 mov ax,si
		 sub ax,1
		 ;1~2880 1~36 * 80
		 ;0~2879
		 mov cl,36
		 div cl
		 ;余数0 eg:36 柱面数1 余数换算为0面1扇区
	     mov ch,al
		 
		 mov cl,8 ;右移的量需要存放在cl中
		 shr ax,cl ;ah部分（余数）是新的除数
		; mov ax,ah
		 mov cl,18
		 div cl ;商 = 磁头号 0/1 余数+1 = 扇区号
		 mov dh,al
		 mov cl,ah
		 add cl,1
	;read 开始 es做内存段寄存器
    read：
         mov ah,0x02
		 mov al,1
		 mov dl,0x00
		 INT 0x13
		 JC err ;出错了 重读一次 也不要重读太多 
		 ;hlt
		 err:
		 mov ax,0xb800                 ;指向文本模式的显示缓冲区
         mov es,ax ;以下显示字符串"Label offset:"
 mov byte [es:0x00],'o'
 mov byte [es:0x01],0x07
 mov byte [es:0x02],'v'
 mov byte [es:0x03],0x07
 mov byte [es:0x04],'e'
 mov byte [es:0x05],0x07
 mov byte [es:0x06],'r'
 mov byte [es:0x07],0x07	
;--------------------------------------------------------		 
phy_base: 
  dd 0x10000

  RESB  0x1fe - ($ - $$)  ; 填写0x00，直到0x001fe
  DB    0x55, 0xaa
