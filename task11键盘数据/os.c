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
  
  //extern struct FIFIO8 keyinfo;//data段
  unsigned char keybuf[32];
  extern struct FIFO8 *keyinfo;
  fifo8_init(keyinfo,32,keybuf);
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
  //init_keyboard();
 // enable_mouse();
//既然是跳过去执行 应该有执行权限
  io_sti();
  int debug_position = 1;
 		for (;;) {
       if(fifo8_status(keyinfo) > 0){
         char *tmp = "  ";
         
         sprintf(tmp,"%d",fifo8_status(keyinfo));
         fifo8_get(keyinfo);
	       putfont8_asc(binfo->vram, binfo->scrnx, 8,16*debug_position, ROSE,(unsigned char*)tmp);	
	       debug_position++;
         io_hlt();
       }else{
        io_hlt();
       }
}


}
