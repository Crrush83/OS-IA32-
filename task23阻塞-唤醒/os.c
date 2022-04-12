#include "io.h"
#include "interupt.h"
#include "time.h"
#include "keyboardmouse.h"
#include <stdio.h>
#include "fifo.h"
#include "memman.h"
#include "graphic.h"
#include "layer.h"
#include "list.h"
#include "multitask.h"
#define BINFOADDR 0x0ff0
#define CLOCKADDR 0x0ffc //8个字符 结尾占1
#define TIMERADDR 0x1010
struct BOOTINFO *binfo;
struct MEMMAN *memman;
extern struct LAYERMAN *layman;
struct INTERUPT_GATE_DESCRIPTOR *idt;
int mousex = 0,mousey = 0;
struct LAYER *timelayer;
struct TIMERMAN *timerman;
struct FIFO32 *keyfifo; 
struct FIFO32 *mousefifo; 
int cursor_x0,cursor_x,cursor_y0 ,cursor_y,cursor_c;//文字框起始于4 20
int main(void){
  binfo = (struct BOOTINFO*)0x0ff0;
//内存分布图0x0ff0开始应该顺次填入BOOTINFO的内容
  memman= (struct MEMMAN *)MEMMAN_ADDR;
  idt = (struct INTERUPT_GATE_DESCRIPTOR*)0x17e00;
  char line[40];//debug
 

  struct MOUSE_DEC mdec;
  init_mouse_decode(&mdec);//原来的位置
  /*memmory managment*/

  memman_init(memman);
  int memsize = memtest(0x130000, 0x212ffff)/(1024*1024);//2000000B = 0x20MB = 32MB//怎么输出得48
  sprintf(line,"%d MB",memsize); 
 // debugPrint((unsigned char *)line);
    //向memman注册32MB
  memman_free(0x130000,1024*1024*32);
  /*fifo*/
  //unsigned char keybuf[32];
  int *keybuf = (int*)memman_alloc(32*4);
  int *mousebuf = (int *)memman_alloc(32*4);
  keyfifo = (struct FIFO32*)memman_alloc(sizeof(struct FIFO32));
  mousefifo = (struct FIFO32*)memman_alloc(sizeof(struct FIFO32));
  fifo32_init(keyfifo,32,keybuf); 
  fifo32_init(mousefifo,32,mousebuf);
  extern unsigned char keymap[0x54];

	init_palette();

  /*fifo malloc*/
  //timer fifo
    unsigned char *timerbuf = (unsigned char *)memman_alloc(32);
    extern struct FIFO32 *timerfifo;
    timerfifo = (struct FIFO32 *)memman_alloc(sizeof(struct FIFO32)); 
    fifo32_init(timerfifo,32,timerbuf);

  /*layer managment*/
  layman = layerman_init(memman,binfo);
  extern struct LAYER* backgroundlayer;
  backgroundlayer = layer_screen(320,200);
  cursor_x0 = 7;cursor_x = 0;cursor_y0 = 23;cursor_y = 0;cursor_c = BLACK;//文字框起始于4 20
  struct LAYER *winl = layer_window(20,20,200,100);
  timelayer = layer_time();
  struct LAYER *mouselayer = layer_mouse(16,16);
  /*list可以使用*/
  /*timer management*/
  /*
  */
  timerman = timerman_init();
  io_cli();
	init_idt();
  //install clock interupt
  install_clock_int();
  //install mouse interupt
  set_intgatedesc(idt+0x74,(int)int0x74,0x30,0x8e00); 
  //install kb interupt
  set_intgatedesc(idt+0x21,(int)int0x21,0x30,0x8e00); 
  initRTC();
  init8259A();
  init_keyboard();// 键盘OK 
  enable_mouse();//单独enable mouse mouse是可以了键盘却关了
//既然是跳过去执行 应该有执行权限
/*task controller*/
    task_init();
    struct TASK *task_b = task_alloc();
    task_b->tss.esp = memman_alloc_4k(64 * 1024) + 64 * 1024;//栈空间 返回高地址！
    task_b->tss.eflags = 0x00000202;
	  task_b->tss.eip = (int)&task_b_main;
  	task_b->tss.es = 0x28;
  	task_b->tss.cs = 0x30;
  	task_b->tss.ss = 0x28;
  	task_b->tss.ds = 0x28;
  	task_b->tss.fs = 0x28;
  	task_b->tss.gs = 0x28;
    task_run(task_b);

     io_sti();//开放中断的时刻可能出现任务切换！即定时器触发的

    new_timer(CURSOR_COL_SWAP_GAP,'c',timerfifo);
  //系统挂钟开始运行！S
    int mycount = 0;
 // char scount[32];
 		for (;;) {
     //  sendEOI();
       mycount++;
       io_cli();
       if(fifo32_status(keyfifo) + fifo32_status(mousefifo) + fifo32_status(timerfifo) > 0){
         if(fifo32_status(mousefifo) > 0){
        //get data then clean int 不论使用什么样的类型标识 只是影响打印的值 存储的二进制不会变化
        unsigned char mousedata = fifo32_get(mousefifo);
         io_sti();
         if(mouse_decode(&mdec,mousedata)!=0){
          // char *tmp = "         ";
          // sprintf(tmp,"x:%d y:%d",mdec.x,mdec.y);
          // debugPrint((unsigned char*)tmp);
          move_mouse(binfo,&mdec,&mousex,&mousey);
          layer_slide(mouselayer,mousex,mousey);
          //mouse slide是否暗含窗口的移动？
          if(mdec.btn & 0x01){
            //按下左键
            layer_slide(winl,mousex-80,mousey - 8);
          }
          }
        }
         else if(fifo32_status(keyfifo) > 0){
            unsigned char key = fifo32_get(keyfifo);
            io_sti();
            if(key < 0x54){
            if(keymap[key]!=0){//可现实字符
            char value[2];
            value[0] = keymap[key];
            value[1] = 0;
            putstr_on_layer(winl,cursor_x0+cursor_x,cursor_y0+cursor_y,BLACK,WHITE,value,1);
            cursor_x += 8;}
            }
            //相对文本框的位置大于7
            if(key == 0x0e && cursor_x > 7){
              //退格键 光标位置用空格键涂抹
              putstr_on_layer(winl,cursor_x0+cursor_x,cursor_y0+cursor_y,BLACK,WHITE," ",1);
              cursor_x -= 8;
            }
            //不管是打字 还是删除 都要在新的位置显示光标
            putcursor_on_layer(winl,cursor_x0+cursor_x,cursor_y0+cursor_y,cursor_c);
         }else if(fifo32_status(timerfifo) > 0){
             unsigned char tid = fifo32_get(timerfifo);
             io_sti();
             if(tid == (unsigned char)'c'){
              swap_cursor_color();//为什么只处理了一次呢？ 
              char scount[32];   
              putcursor_on_layer(winl,cursor_x0+cursor_x,cursor_y0+cursor_y,cursor_c);//
             }else{
               //td == 's'
             }
         }
       }else{
        io_sti();
        io_hlt();
       }
}
return 0;
}