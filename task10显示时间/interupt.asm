[BITS 32]
GLOBAL init8259A,int0x70,int0x21,int0x74
GLOBAL load_idtr,exception

EXTERN showtime,readtime,exceptionprint,showmouse,showkb

[SECTION .TEXT]
load_idtr:
    MOV AX,[ESP+4]		; limit 只需要16位
    MOV		[ESP+6],AX
	LIDT	[ESP+6] ;紧跟48位 低16是界限 高32是起始位置
	RET
init8259A:
        cli
        ;丛片的中断掩码
         mov al,0xee    ;11101110 1是禁止中断 0是打开中断       鼠标&周期结束             ;清除bit 0(此位连接RTC)
         out 0xa1,al                        ;写回此寄存器
        ;暂时禁掉主片所有中断
        ;主片掩码端口是0x0021
         mov al,0xfd ;11111101 键盘
         out 0x21,al
         
         mov al,0x11
         out 0x20,al                        ;ICW1：边沿触发/级联方式
         mov al,0x20
         out 0x21,al                        ;ICW2:起始中断向量
         mov al,0x04
         out 0x21,al                        ;ICW3:从片级联到IR2
         mov al,0x01
         out 0x21,al                        ;ICW4:非总线缓冲，全嵌套，正常EOI

         mov al,0x11
         out 0xa0,al                        ;ICW1：边沿触发/级联方式
         mov al,0x70
         out 0xa1,al                        ;ICW2:起始中断向量
         mov al,0x02
         out 0xa1,al                        ;ICW3:从片级联到IR2
         mov al,0x01
         out 0xa1,al                        ;ICW4:非总线缓冲，全嵌套，正常EOI
                                ;读8259从片的IMR寄存器
         sti
         ret
  ;time interupt
  int0x70:
      pushad
      mov eax,esp
      push eax
;ss不会在中断里改变吧？
      call readtime
      call showtime
      ;手动结束中断
      ;同时给主片和丛片汇报EOI
      mov al,0x20
      out 0x20,al
      out 0xa0,al
      pop eax
      popad

      IRETD ;原来的cs和ip保存下来了吗？
 int0x74:
    ;鼠标中断
     pushad
      mov eax,esp
      push eax
;ss不会在中断里改变吧？
      call showmouse
      ;手动结束中断
      ;同时给主片和丛片汇报EOI
      mov al,0x20
      out 0x20,al
      out 0xa0,al
      pop eax
      popad

      IRETD ;原来的cs和ip保存下来了吗？
  int0x21:
    ;键盘中断
       pushad
      mov eax,esp
      push eax
;ss不会在中断里改变吧？
      call showkb
      ;手动结束中断
      ;同时给主片和丛片汇报EOI
      mov al,0x20
      out 0x20,al
      out 0xa0,al
      pop eax
      popad

      IRETD ;原来的cs和ip保存下来了吗？     
    
      
 exception:
      pushad
      call exceptionprint
      popad
      mov al,0x20
      out 0x20,al
      out 0xa0,al
    IRETD