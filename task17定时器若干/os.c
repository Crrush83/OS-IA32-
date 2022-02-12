#include "io.h"
#include "interupt.h"
#include "time.h"
#include "keyboardmouse.h"
#include <stdio.h>
#include "fifo.h"
#include "memman.h"
#include "graphic.h"
#include "layer.h"
#define BINFOADDR 0x0ff0
#define CLOCKADDR 0x0ffc //8个字符 结尾占1
#define TIMERADDR 0x1010
struct BOOTINFO *binfo;
struct MEMMAN *memman;
struct INTERUPT_GATE_DESCRIPTOR *idt;
int mousex = 0,mousey = 0;
struct LAYER *timelayer;
int main(void){
  binfo = (struct BOOTINFO*)0x0ff0;
//内存分布图0x0ff0开始应该顺次填入BOOTINFO的内容
  memman= (struct MEMMAN *)MEMMAN_ADDR;
  idt = (struct INTERUPT_GATE_DESCRIPTOR*)0x17e00;
  char line[40];//debug
  unsigned char keybuf[32];
  unsigned char mousebuf[32];
  extern struct FIFO8 *keyfifo; 
  fifo8_init(keyfifo,32,keybuf); 
  extern struct FIFO8 *mousefifo; 
  fifo8_init(mousefifo,32,mousebuf);
  struct MOUSE_DEC mdec;
  init_mouse_decode(&mdec);//原来的位置
  /*memmory managment*/

  memman_init(memman);
  int memsize = memtest(0x130000, 0x212ffff)/(1024*1024);//2000000B = 0x20MB = 32MB//怎么输出得48
  sprintf(line,"%d MB",memsize); 
  debugPrint((unsigned char *)line);
    //向memman注册32MB
  memman_free(memman,0x130000,1024*1024*32);

	init_palette();
  //debugPrint("why Broken");
  /*layer managment*/
  extern struct LAYERMAN *layman; 
  layman = layerman_init(memman,binfo);
  layer_screen(320,200);
  layer_window(20,20,200,100);
  timelayer = layer_time();
  struct LAYER *mouselayer = layer_mouse(16,16);
 // debugPrint("rstuvwxyz");
  int i;

  io_cli();
  int *countaddr = TIMERADDR;
  *(countaddr) = 0; 
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
 		for (;;) {
       io_cli();
       if(fifo8_status(keyfifo) + fifo8_status(mousefifo) > 0){
         if(fifo8_status(mousefifo) > 0){
        //get data then clean int 不论使用什么样的类型标识 只是影响打印的值 存储的二进制不会变化
        unsigned char mousedata = fifo8_get(mousefifo);
         io_sti();
         if(mouse_decode(&mdec,mousedata)!=0){
          // char *tmp = "         ";
          // sprintf(tmp,"x:%d y:%d",mdec.x,mdec.y);
          // debugPrint((unsigned char*)tmp);
          move_mouse(binfo,&mdec,&mousex,&mousey);
          layer_slide(layman,mouselayer,mousex,mousey);
          }
        }
         else{
        //  char *tmp = "    ";
        //  sprintf(tmp,"%d",fifo8_get(keyfifo));
        //  debugPrint((unsigned char *)tmp);
        fifo8_get(keyfifo);
         io_sti();
         }
       }else{
        io_sti();
        io_hlt();
       }
}
return 0;
}