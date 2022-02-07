#include "io.h"
#include "interupt.h"
#include "time.h"
#include "keyboardmouse.h"
#include <stdio.h>
#include "fifo.h"
#include "memman.h"
// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
//全局变量在.data段中 在链接文件的末尾
struct BOOTINFO *binfo = (struct BOOTINFO*)0x0ff0;//内存分布图0x0ff0开始应该顺次填入BOOTINFO的内容
//00fc:内存管理
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
  char line[40];//debug
  //extern struct FIFIO8 keyfifo;//data段
  unsigned char keybuf[32];
  unsigned char mousebuf[32];
  extern struct FIFO8 *keyfifo;  
  extern struct FIFO8 *mousefifo;  
  fifo8_init(keyfifo,32,keybuf);
  fifo8_init(mousefifo,32,mousebuf);
  struct MOUSE_DEC mdec;
  init_mouse_decode(&mdec);
  int mousex = 151,mousey = 91;
  int i;
  for(i = 0;i < 1024;i++){
        clockbg[i] = MOON;
  }
  


	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	//show a string
 // putfont8_asc(binfo->vram, binfo->scrnx, 8, 8, ROSE, (unsigned char*)"show string API");
	//show a cursor!
	init_mouse_cursor8(mcursor,BABYBLUE);
	putblockVRAM(binfo->vram,binfo->scrnx,16,16,mousex,mousey,mcursor,16);
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
  int debug = 1;
  /*memman:test memory*/
  unsigned int memsize = 0;
  //二进制1000对齐 差是 xxx7 或者000f
 
 //memsize = memtest(0xfe000000, 0xffffffff)/(1024*1024);//2000000B = 0x20MB = 32MB//怎么输出得48
 //终于对了一次
    struct MEMMAN *memory_manager = (struct MEMMAN *)MEMMAN_ADDR;
    memman_init(memory_manager);
    
    /*check memory*/
    unsigned memtotal;
    memsize = memtest(0x120000, 0x211ffff)/(1024*1024);//2000000B = 0x20MB = 32MB//怎么输出得48
    sprintf(line,"%d MB",memsize); 
    debugPrint((unsigned char *)line);
    //向memman注册32MB
    memman_free(memory_manager,0x120000,1024*1024*32);
    // memman_alloc(memory_manager,1024*1024*2);//2MB
    // sprintf(line,"free: %dMB",memman_total(memory_manager)/(1024*1024));
    // debugPrint(line);

 		for (;;) {
       io_cli();
       if(fifo8_status(keyfifo) + fifo8_status(mousefifo) > 0){
         if(fifo8_status(mousefifo) > 0){
        //get data then clean int 不论使用什么样的类型标识 只是影响打印的值 存储的二进制不会变化
        unsigned char mousedata = fifo8_get(mousefifo);
         io_sti();
         if(mouse_decode(&mdec,mousedata)!=0){
        //鼠标全局变量 mousex mousey
        //计算新坐标 隐去旧鼠标 描画新鼠标
          box_fill8(binfo->vram, binfo->scrnx, BABYBLUE, mousex, mousey, mousex+15, mousey+15);
          move_mouse(binfo,&mdec,&mousex,&mousey);//修改鼠标背景 修改鼠标坐标
        	putblockVRAM(binfo->vram,binfo->scrnx,16,16,mousex,mousey,mcursor,16);
          }
        }
         else{
         char *tmp = "    ";
         sprintf(tmp,"%d",fifo8_get(keyfifo));
         io_sti();
         }
       }else{
        io_sti();
        io_hlt();
       }
}


}
