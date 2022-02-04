;[FORMAT "WCOFF"]
[BITS 32]
GLOBAL io_hlt
[SECTION .text]
io_hlt:
    MOV EAX,0x00000000
	MOV EAX,0x11111111
	MOV EAX,0x00000000
    RET
    
    