; hello-os

  ORG   0x7c00            ; 指明程序的装载地址

; 用于标准FAT12格式的软盘
  JMP   entry             ; 跳转指令
  NOP                     ; NOP指令（0x90）
  DB    "HELLOIPL"        ; OEM标识符（8字节）
  DW    512               ; 每个扇区（sector）的字节数（必须为512字节）
  DB    1                 ; 每个簇（cluster）的扇区数（必须为1个扇区）
  DW    1                 ; FAT的预留扇区数（包含boot扇区）
  DB    2                 ; FAT表的数量，通常为2
  DW    224               ; 根目录文件的最大值（一般设为224项）
  DW    2880              ; 磁盘的扇区总数，若为0则代表超过65535个扇区，需要使用20行记录
  DB    0xf0              ; 磁盘的种类（本项目中设为0xf0代表1.44MB的软盘）
  DW    9                 ; 每个FAT的长度（必须为9扇区）
  DW    18                ; 1个磁道（track）拥有的扇区数（必须是18）
  DW    2                 ; 磁头数（必须为2）
  DD    0                 ; 隐藏的扇区数
  DD    2880              ; 大容量扇区总数，若14行记录的值为0则使用本行记录扇区数
  DB    0                 ; 中断0x13的设备号
  DB    0                 ; Windows NT标识符
  DB    0x29              ; 扩展引导标识
  DD    0xffffffff        ; 卷序列号
  DB    "HELLO-OS   "     ; 卷标（11字节）
  DB    "FAT12   "        ; 文件系统类型（8字节）
  RESB  18                ; 空18字节

; 程序核心

entry:

 mov ax,0xb800                 ;指向文本模式的显示缓冲区
 mov es,ax ;以下显示字符串"Label offset:"
 mov byte [es:0x00],'L'
 mov byte [es:0x01],0x07
 mov byte [es:0x02],'a'
 mov byte [es:0x03],0x07
 mov byte [es:0x04],'b'
 mov byte [es:0x05],0x07
 mov byte [es:0x06],'e'
 mov byte [es:0x07],0x07
 mov byte [es:0x08],'l'
 mov byte [es:0x09],0x07
 mov byte [es:0x0a],' '
 mov byte [es:0x0b],0x07
 mov byte [es:0x0c],"o"
 mov byte [es:0x0d],0x07
 mov byte [es:0x0e],'f'
 mov byte [es:0x0f],0x07
 mov byte [es:0x10],'f'
 mov byte [es:0x11],0x07
 mov byte [es:0x12],'s'
 mov byte [es:0x13],0x07
 mov byte [es:0x14],'e'
 mov byte [es:0x15],0x07
 mov byte [es:0x16],'t'
 mov byte [es:0x17],0x07
 mov byte [es:0x18],':'
 mov byte [es:0x19],0x07
;fin:
 ; HLT                     ; CPU停止，等待指令
 ; JMP   fin               ; 无限循环

  RESB  0x1fe - ($ - $$)  ; 填写0x00，直到0x001fe
  DB    0x55, 0xaa
