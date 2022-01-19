 [BITS 32]
 GLOBAL load_idtr
 
 load_idtr:
    MOV AX,[ESP+4]		; limit 只需要16位
    MOV		[ESP+6],AX
	LIDT	[ESP+6] ;紧跟48位 低16是界限 高32是起始位置
	RET