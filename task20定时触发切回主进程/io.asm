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
  GLOBAL load_cr0,store_cr0
  GLOBAL write_mem8,write_mem32
  GLOBAL memtest_sub_asm
  GLOBAL load_tr,task_switch,farjmp8



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

io_out8:                ; void io_out8(int port, int data);;在函数内esp为空esp+4为最后一个压入参数的首字节
;（即为传进去的第一个参数）
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
load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

write_mem8:             ; void write_mem8(int addr, int data);
  MOV     ECX, [ESP+4]  ; addr
   MOV     AL, [ESP+8]   ; data
   MOV     [ECX], AL
   RET
write_mem32:
   MOV     ECX, [ESP+4]  ; addr
   MOV     eax, [ESP+8]   ; data
   MOV     [ECX], eax
   RET

;unsigned int memtest_sub_asm(unsigned int start, unsigned int end); 
memtest_sub_asm:
		PUSH	EDI						; （EBX, ESI, EDI も使いたいので）
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;+12的解释 逆序压入参数 返回地址 返回值 
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
	;	JNE		mts_fin
		JNE		setneq ;怎么会写不成功呢？
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
    CMP  EAX,0xfffff000
    JE mts_fin
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
    setneq:
    MOV EAX,0x1234abcd
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET
load_tr:
  LTR [esp+4]
  RET

task_switch: ;(offset)
  jmp far [esp] 
  RET

farjmp8:
  jmp 0x40:0x00
  RET
