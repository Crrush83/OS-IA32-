#include "io.h"
#include "interupt.h"
#include "time.h"
#include "keyboardmouse.h"
#include <stdio.h>
#include "fifo.h"
// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
//全局变量在.data段中 在链接文件的末尾
struct BOOTINFO *binfo = (struct BOOTINFO*)0x0ff0;//内存分布图0x0ff0开始应该顺次填入BOOTINFO的内容
struct INTERUPT_GATE_DESCRIPTOR *idt = (struct INTERUPT_GATE_DESCRIPTOR*)0x17e00;
//struct SEGMENT_GATE_DESCRIPTOR *gdt = (struct INTERUPT_GATE_DESCRIPTOR*)0x17e00;
char clockbg[1024];
char mcursor[256];
//函数调用的原理
//保存下条指令的地址
//参数压栈
//修改ip 函数的return 暗含设置返回值寄存器和修改ip的意思
//不关心现在的CS是什么 既然调用存在一个段中 修改ip+=两条指令的差值
//那么汇编指令的CALL呢？
//中断程序却可能修改CS和段内偏移（需要绝对地址和段起始值换算）
//
int main(void) {
  
 // extern char hankaku[4096];
  
  //extern struct FIFIO8 keyfifo;//data段
  unsigned char keybuf[32];
  unsigned char mousebuf[32];
  extern struct FIFO8 *keyfifo;  
  extern struct FIFO8 *mousefifo;  
  fifo8_init(keyfifo,32,keybuf);
  fifo8_init(mousefifo,32,mousebuf);
  struct MOUSE_DEC mdec;
  int i;
  for(i = 0;i < 1024;i++){
        clockbg[i] = MOON;
  }
  
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	//show a string
 // putfont8_asc(binfo->vram, binfo->scrnx, 8, 8, ROSE, (unsigned char*)"show string API");
	//show a cursor!
	init_mouse_cursor8(mcursor,BABYBLUE);
	putblockVRAM(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
  //clock background
  //set idt
  io_cli();
	init_idt();
  install_clock_int();
  
  set_intgatedesc(idt+0x74,(int)int0x74,0x30,0x8e00); 
  //install keyboard int
  set_intgatedesc(idt+0x21,(int)int0x21,0x30,0x8e00); 
  initRTC();
  init8259A();
  init_keyboard();// 键盘OK 
  enable_mouse();//单独enable mouse mouse是可以了键盘却关了
//既然是跳过去执行 应该有执行权限
  io_sti();
  //int debug_position = 1;
  //int kb;
 		for (;;) {
       if(fifo8_status(keyfifo) + fifo8_status(mousefifo) > 0){
         if(fifo8_status(mousefifo) > 0){
         char *tmp = "    ";
         sprintf(tmp,"%d",fifo8_get(mousefifo));
         box_fill8(binfo->vram,binfo->scrnx,BABYBLUE,8,16,23,31);
	       putfont8_asc(binfo->vram, binfo->scrnx, 8,16, ROSE,(unsigned char*)tmp);	
         }
         else{
         char *tmp = "    ";
         sprintf(tmp,"%d",fifo8_get(keyfifo));
         box_fill8(binfo->vram,binfo->scrnx,BABYBLUE,8,32,23,63);
	       putfont8_asc(binfo->vram, binfo->scrnx, 8,32, ROSE,(unsigned char*)tmp);	
         //io_hlt();
         }
       }else{
        io_hlt();
       }
}


}
