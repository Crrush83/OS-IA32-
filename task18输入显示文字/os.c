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
struct FIFO8 *keyfifo; 
struct FIFO8 *mousefifo; 
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
  debugPrint((unsigned char *)line);
    //向memman注册32MB
  memman_free(0x130000,1024*1024*32);
  /*fifo*/
  //unsigned char keybuf[32];
  unsigned char *keybuf = (unsigned char *)memman_alloc(32);
  unsigned char *mousebuf = (unsigned char *)memman_alloc(32);
  keyfifo = (struct FIFO8*)memman_alloc(sizeof(struct FIFO8));
  mousefifo = (struct FIFO8*)memman_alloc(sizeof(struct FIFO8));
  fifo8_init(keyfifo,32,keybuf); 
  fifo8_init(mousefifo,32,mousebuf);
  extern unsigned char keymap[0x54];

	init_palette();
  //debugPrint("why Broken");

  /*fifo malloc*/
  //timer fifo
    unsigned char *timerbuf = (unsigned char *)memman_alloc(32);
    extern struct FIFO8 *timerfifo;
    timerfifo = (struct FIFO8 *)memman_alloc(sizeof(struct FIFO8)); 
    fifo8_init(timerfifo,32,timerbuf);
  /*layer managment*/
  layman = layerman_init(memman,binfo);
  layer_screen(320,200);
  cursor_x0 = 7;cursor_x = 0;cursor_y0 = 23;cursor_y = 0;cursor_c = BLACK;//文字框起始于4 20
  struct LAYER *winl = layer_window(20,20,200,100);
  timelayer = layer_time();
  struct LAYER *mouselayer = layer_mouse(16,16);
  /*list可以使用*/
  /*timer management*/
  /*
  */
  timerman = timerman_init();
  //光标定时器 0.5s
  new_timer(2,'w');//定时器提醒我我就换成白的
  

  //查看链表
  // unsigned int sec = sectimeout(timerman);
  // sprintf(line,"%d addr",sec); 
  // debugPrint((unsigned char *)line);

 // debugPrint("rstuvwxyz");
  int i;

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

  
 		for (;;) {
       io_cli();
       if(fifo8_status(keyfifo) + fifo8_status(mousefifo) + fifo8_status(timerfifo) > 0){
         if(fifo8_status(mousefifo) > 0){
        //get data then clean int 不论使用什么样的类型标识 只是影响打印的值 存储的二进制不会变化
        unsigned char mousedata = fifo8_get(mousefifo);
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
         else if(fifo8_status(keyfifo) > 0){
            unsigned char key = fifo8_get(keyfifo);
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
         }else if(fifo8_status(timerfifo) > 0){
             unsigned char tid = fifo8_get(timerfifo);
              io_sti();

             if(tid == (unsigned char)'b' || tid == (unsigned char)'w'){
              swap_cursor_color(tid);//为什么只处理了一次呢？    
              putcursor_on_layer(winl,cursor_x0+cursor_x,cursor_y0+cursor_y,cursor_c);
             }
            
         }
       }else{
        io_sti();
        io_hlt();
       }
}
return 0;
}