[BITS 32]
GLOBAL init8259A
EXTERN int0x70
[SECTION .TEXT]
init8259A:
        cli
        ;丛片的中断掩码
         mov al,0xfe    ;11111110 1是禁止中断 0是打开中断                    ;清除bit 0(此位连接RTC)
         out 0xa1,al                        ;写回此寄存器
        ;暂时禁掉主片所有中断
        ;主片掩码端口是0x0021
         mov al,0xff
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
 