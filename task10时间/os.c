#include "name.h"
#include "interupt.h"
#include "time.h"

// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
//全局变量在.data段中 在链接文件的末尾
struct BOOTINFO *binfo = (struct BOOTINFO*)0x0ff0;//内存分布图0x0ff0开始应该顺次填入BOOTINFO的内容
struct INTERUPT_GATE_DESCRIPTOR *idt = (struct INTERUPT_GATE_DESCRIPTOR*)0x17e00;
char clockbg[1024];

int main(void) {
 // extern char hankaku[4096];
    char mcursor[256];

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
//  installRTC();
	//且标号的偏移值相对（）？
	//是链接文件开始的0x100000吗？
	//main函数一定要从0x100000开始
	//段选择器 + 段偏移换算后的那条指令就是0x100000 
	//我想问 程序里的jmp xx指令 是相对0x100000的吗？
  //设置0x70中断
  install_clock_int();
  set_intgatedesc(idt+0x74,(int)int0x74,0x30,0x8e00); //这里不能强转哦
  set_intgatedesc(idt+0x21,(int)int0x21,0x30,0x8e00); //这里不能强转哦

  initRTC();
  init8259A();

 
//既然是跳过去执行 应该有执行权限
  
  io_sti();
 	 for (;;) {
    io_hlt();
  }
}



