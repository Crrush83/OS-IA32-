;[FORMAT "WCOFF"]
;定义一些常数 关于现在选择的显示模式 大小 色彩位数？
;现存开始的地方？ 现在是0xa0000 但是从哪里得到这个数值呢？

; 有关BOOT_INFO
CYLS    EQU   0x0ff0      ; 设置启动区
LEDS    EQU   0x0ff1
VMODE   EQU   0x0ff2      ; 关于颜色数目的信息，颜色的位数
SCRNX   EQU   0x0ff4      ; 分辨率X
SCRNY   EQU   0x0ff6      ; 分辨率Y
VRAM    EQU   0x0ff8      ; 图像缓冲区的起始位置


 ; [FORMAT "WCOFF"]
  [BITS 32]

  ; 程序中包含函数名
  GLOBAL  io_cli,io_hlt, io_sti, io_stihlt
  GLOBAL io_in8, io_in16, io_in32
  GLOBAL io_out8, io_out16, io_out32
  GLOBAL io_load_eflags, io_store_eflags
   GLOBAL write_mem8

  [SECTION .text]
  io_hlt:                 ; void io_hlt(void);
  HLT
  RET
  io_cli:                 ; void io_cli(void);
  CLI
  RET
io_sti:                 ; void io_sti(void);
  STI
  RET

io_stihlt:              ; void io_stihlt(void);
  STI
  HLT
  RET

io_in8:                 ; int io_in8(int port);
  MOV     EDX, [ESP+4]  ; port
  MOV     EAX, 0
  IN      AL, DX
  RET

io_in16:                ; int io_in16(int port);
  MOV     EDX, [ESP+4]  ; port
  MOV     EAX, 0
  IN      AX, DX
  RET

io_in32:                ; int io_in32(int port);
  MOV     EDX, [ESP+4]  ; port
  IN      EAX, DX
  RET

io_out8:                ; void io_out8(int port, int data);
  MOV     EDX, [ESP+4]  ; port
  MOV     AL, [ESP+8]   ; data
  OUT     DX, AL
  RET

io_out16:               ; void io_out16(int port, int data);
  MOV     EDX, [ESP+4]  ; port
  MOV     AX, [ESP+8]   ; data
  OUT     DX, AX
  RET

io_out32:               ; void io_out32(int port, int data);
  MOV     EDX, [ESP+4]  ; port
  MOV     EAX, [ESP+8]  ; data
  OUT     DX, EAX
  RET

io_load_eflags:         ; int io_load_eflags(void);
  PUSHFD
  POP     EAX
  RET

io_store_eflags:        ; void io_store_eflags(int eflags);
  MOV     EAX, [ESP+4]  ; eflags
  PUSH    EAX
  POPFD
  RET

 write_mem8:             ; void write_mem8(int addr, int data);
  MOV     ECX, [ESP+4]  ; addr
   MOV     AL, [ESP+8]   ; data
   MOV     [ECX], AL
   RET