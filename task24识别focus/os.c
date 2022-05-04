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
#include "focus.h"
#define BINFOADDR 0x0ff0
#define CLOCKADDR 0x0ffc //8个字符 结尾占1
#define TIMERADDR 0x1010
struct BOOTINFO *binfo;
struct MEMMAN *memman;
extern struct LAYERMAN *layman;
struct INTERUPT_GATE_DESCRIPTOR *idt;
//转移到keyboardmouse里
int mousex = 0,mousey = 0;
//focus在背景上
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

  memman_free(0x130000,1024*1024*32);
  /*fifo*/
  //unsigned char keybuf[32];
  int *keybuf = (int*)memman_alloc(32*4);
  int *mousebuf = (int *)memman_alloc(32*4);
  //交互keyfifo 未来的命令行输入 和taska互相绑定
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
  extern focus_layer;
  focus_layer = backgroundlayer;
  extern focus_task;
 // focus_task = NULL;
  cursor_x0 = 7;cursor_x = 0;cursor_y0 = 23;cursor_y = 0;cursor_c = BLACK;//文字框起始于4 20
  
  
  timelayer = layer_time();
  struct LAYER *mouselayer = layer_mouse(16,16);
  //   char focus_debug[100];
  // sprintf(focus_debug,"mouse addr %d",(int)mouselayer);
  // debugPrint(focus_debug);
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
    //main task
    struct TASK *task_a = task_init();
    //已经包含task_a加入run队列
    //绘制window_a 怎么显示不出来新窗口然后中断没了？判断是内存出错
    //怎么改了根没改一样？我的sychronize 一关机就挂了
    //怎么现在qemu可以有输入和鼠标移动 bochs不可了？看一下任务focus对了没 买点零食去
    struct LAYER *window_a = layer_register();
    unsigned char *win_buf_a = (unsigned char *)memman_alloc_4k(114*52);
    layer_setbuf(window_a,win_buf_a,144,52,-1);
    layer_settitle(window_a,(unsigned char*)"task_main");
    draw_window(window_a->buf,144,52,window_a->title,0);
    make_textbox8(window_a,4,20,110,50,WHITE);
    window_a->task = task_a;//只有任务a的窗口聚焦的时候 才看得到输入
    task_a->bind_layer = window_a;
    task_a->keyfifo = keyfifo;
    fifo32_bind_task(timerfifo,task_a);//timerfifo会唤醒taska
    fifo32_bind_task(keyfifo,task_a);//keyfifo会唤醒taska 同时只有taska被鼠标聚焦的时候keyfifo才会填入数据 因为taska也绑定了keyfifo
   //在window a是focuslayer 所以taskmain是focustask的时候 输入才在命令框。。
    //fifo32_bind_task(mousefifo,task_a);

    //taskb
    struct TASK *task_b[3];//再来三个任务 关联三个窗口 显示点击效果
    struct LAYER *window_b[3];//指针的数组
    unsigned  char * buf_win_b[3];//这三个图层绑定的显存副本
    /* window_b */
    int i;
	  for (i = 0; i < 3; i++) {
		window_b[i] = layer_register();//注册图层 然后为buf分配位置 填充/绑定
		buf_win_b[i] = (unsigned char *) memman_alloc_4k(144 * 52);
    layer_setbuf(window_b[i],buf_win_b[i],144,52,-1);
    char s[30];
  	sprintf(s, "task_b%d", i);
    layer_settitle(window_b[i],(unsigned char*)s);
		draw_window(window_b[i]->buf, 144, 52, window_b[i]->title, 0);
    //绘制窗口显存副本完成 创建任务
		task_b[i] = task_alloc();
    task_b[i]->tss.eflags = 0x00000202;
		task_b[i]->tss.esp = memman_alloc_4k( 64 * 1024) + 64 * 1024 - 8;//因为已经传入一个参数了呢 所以要-8
    //esp：空 esp+4 : 第一个参数 esp+8:函数地址
		task_b[i]->tss.eip = (int) &task_b_main;
		task_b[i]->tss.es = 0x28;
		task_b[i]->tss.cs = 0x30;
		task_b[i]->tss.ss = 0x28;
		task_b[i]->tss.ds = 0x28;
		task_b[i]->tss.fs = 0x28;
		task_b[i]->tss.gs = 0x28;
    task_b[i]->bind_layer = window_b[i];
    window_b[i]->task = task_b[i];
    //把
		*(int*)((task_b[i]->tss.esp) + 4) = task_b[i];
		task_run(task_b[i]);//每个b任务运行一样的代码 却有自己的栈
	}//这里是先有任务再让task_a画图

  //将taskb的显存副本移动到现实的位置 指定高度和位置 然后内部调用了refresh
  //background mouse timer 和make window内部已经绘制？
 // layer_slide(sht_back, 0, 0);
	layer_slide(window_b[0], 168,  56);
	layer_slide(window_b[1],   8, 116);
	layer_slide(window_b[2], 168, 116);
	layer_slide(window_a,8,  56);
//	layer_slide(sht_mouse, mx, my);
//	layer_updown(sht_back,     0);
	layer_updown(window_b[0], 1);
	layer_updown(window_b[1], 2);
	layer_updown(window_b[2], 3);
	layer_updown(window_a,      4);
//	layer_updown(sht_mouse,    5);





    //task
    io_sti();//开放中断的时刻可能出现任务切换！即定时器触发的

    new_timer(CURSOR_COL_SWAP_GAP,'c',timerfifo);
  //系统挂钟开始运行！S
    int mycount = 0;
 // char scount[32];
 		for (;;) {
       struct FIFO32* local_keyfifo = task_a->keyfifo;
       sendEOI();
       mycount++;
       io_cli();
       if(fifo32_status(local_keyfifo) + fifo32_status(mousefifo) + fifo32_status(timerfifo) > 0){
         if(fifo32_status(mousefifo) > 0){
        //get data then clean int 不论使用什么样的类型标识 只是影响打印的值 存储的二进制不会变化
        unsigned char mousedata = fifo32_get(mousefifo);
         io_sti();
         if(mouse_decode(&mdec,mousedata)!=0){
           move_mouse(binfo,&mdec,&mousex,&mousey);//改变了Mouse变量 但是没有画出来！
           //可能是单纯的鼠标重绘 也可能是窗口focus的改变
          if(mdec.btn & 0x01){
            //按下左键
            //发生点击 获取点击图层
            /*
            可能改变focus task layer 
            重绘也是中断完成
            改变键盘数据发送的方向也是中断完成
            但是显示新发的数据是任务切换到！
            */
            check_focus(mousex,mousey);
        //    layer_slide(window_a,mousex-80,mousey - 8);
          }
          
          layer_slide(mouselayer,mousex,mousey);
          //mouse slide是否暗含窗口的移动？
          
          }
        }
         else if(fifo32_status(local_keyfifo) > 0){
            unsigned char key = fifo32_get(local_keyfifo);
            io_sti();
            if(key < 0x54){
            if(keymap[key]!=0){//可现实字符
            char value[2];
            value[0] = keymap[key];
            value[1] = 0;
            putstr_on_layer(window_a,cursor_x0+cursor_x,cursor_y0+cursor_y,BLACK,WHITE,value,1);
            cursor_x += 8;}
            }
            //相对文本框的位置大于7
            if(key == 0x0e && cursor_x > 7){
              //退格键 光标位置用空格键涂抹
              putstr_on_layer(window_a,cursor_x0+cursor_x,cursor_y0+cursor_y,BLACK,WHITE," ",1);
              cursor_x -= 8;
            }
            //不管是打字 还是删除 都要在新的位置显示光标
            putcursor_on_layer(window_a,cursor_x0+cursor_x,cursor_y0+cursor_y,cursor_c);
         }else if(fifo32_status(timerfifo) > 0){
             unsigned char tid = fifo32_get(timerfifo);
             io_sti();
             if(tid == (unsigned char)'c'){
              swap_cursor_color();//为什么只处理了一次呢？ 
              char scount[32];   
              putcursor_on_layer(window_a,cursor_x0+cursor_x,cursor_y0+cursor_y,cursor_c);//
             }else{
               //td == 's'
             }
         }
       }else{
        //sleep 改变就绪队列 不允许中断发生
        //task_sleep(task_a);
        io_sti();
       }
}
return 0;
}