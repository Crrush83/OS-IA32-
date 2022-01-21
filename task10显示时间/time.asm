  [BITS 32]
  GLOBAL int0x70,initRTC
  EXTERN showint0x70
  [SECTION .text]
  int0x70:
     ; push es
      ;push ds
     ; pushad
     ; mov eax,esp
     ; push eax
;ss不会在中断里改变吧？
      call showint0x70
      ;手动结束中断
      ;同时给主片和丛片汇报EOI
      mov al,0x20
      out 0x20,al
      out 0xa0,al
      ;pop eax
      ;mov eax
      ;popad
      ;pop ds
      ;pop es
      IRETD ;原来的cs和ip保存下来了吗？
     
      
initRTC:
        ;设置8259A中断控制器
       
         ;mask值现在是什么？
         ;设置和时钟中断相关的硬件 
         mov al,0x0b                        ;RTC寄存器B
         or al,0x80                         ;阻断NMI
         out 0x70,al
         mov al,0x12                        ;设置寄存器B，禁止周期性中断，开放更
         out 0x71,al                        ;新结束后中断，BCD码，24小时制

         
         
         mov al,0x0c    ;向索引端口0x70写入0x0c 就是静态存储电路里的0x0c寄存器 这个写入操作0000 1100 也打开了NMI中断
         ;0x70的最高位连上NMI的与非门
         ;这个值会通过0x71读出
         out 0x70,al
         in al,0x71                         ;读RTC寄存器C，复位未决的中断状态

                              ;开放硬件中断