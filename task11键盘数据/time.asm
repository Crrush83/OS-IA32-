  [BITS 32]
  GLOBAL initRTC,readtime
  [SECTION .text]

     
      
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
ALreadtime:
    mov byte [0xffc],'n'
    mov byte [0xffd],'m'
    mov byte [0xfff],0x00
    ret
readtime:;from RTC
      .w0:                                    
      mov al,0x0a                        ;阻断NMI。当然，通常是不必要的
      or al,0x80                          
      out 0x70,al
      in al,0x71                         ;读寄存器A
      test al,0x80                       ;测试第7位UIP 
      jnz .w0                            ;以上代码对于更新周期结束中断来说 
                                         ;是不必要的 
      xor al,al
      or al,0x80
      out 0x70,al
      in al,0x71                         ;读RTC当前时间(秒)
      push ax

      mov al,2
      or al,0x80
      out 0x70,al
      in al,0x71                         ;读RTC当前时间(分)
      push ax

      mov al,4
      or al,0x80
      out 0x70,al
      in al,0x71                         ;读RTC当前时间(时)
      push ax

      mov al,0x0c                        ;寄存器C的索引。且开放NMI 
      out 0x70,al
      in al,0x71                         ;读一下RTC的寄存器C，否则只发生一次中断
                                         ;此处不考虑闹钟和周期性中断的情况 
      pop ax
      call bcd_to_ascii ;ah 十位 al 个位 我们想要高位在低地址
      mov byte [0xffc],ah
      mov byte [0xffd],al
     

      mov byte [0xffe],':'
      
      pop ax
      call bcd_to_ascii
      mov byte [0xfff],ah
      mov byte [0x1000],al
      mov byte [0x1001],':'

      pop ax
      call bcd_to_ascii
         ;此处不考虑闹钟和周期性中断的情况  
      mov byte [0x1002],ah
      mov byte [0x1003],al      

      mov byte [0x1004],0x00 ;现实的时候 先低地址再高地址 从左向右 反而是低位在前咯
      ;但我们希望高位在前啊
      ret
bcd_to_ascii:                            ;BCD码转ASCII
                                         ;输入：AL=bcd码
                                         ;输出：AX=ascii
                                         ;21 0010 0001 
      mov ah,al                          ;分拆成两个数字 
      and al,0x0f                        ;仅保留低4位 
      add al,0x30                        ;转换成ASCII 

      shr ah,4                           ;逻辑右移4位 
      and ah,0x0f                        
      add ah,0x30

      ret
      