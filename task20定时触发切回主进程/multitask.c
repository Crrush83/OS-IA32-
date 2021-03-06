#include "multitask.h"
#include "time.h"
#include "fifo.h"
#include "io.h"
#include "graphic.h"
#include "stdio.h"

struct TSS32 tss_a, tss_b;

void set_TSSDescriptor(struct TSSDescriptor *tssd, int limit, int base, int ar)
{
    // char s[100],ss[100];
    // sprintf(s,"limit is %d",limit);
    // sprintf(ss,"base is %X",base);
    // debugPrint(ss);
    tssd->limit_low = limit & 0x00ff;
    tssd->base_low = base & 0x0000ffff;
    tssd->base_mid = (base & 0x00ff0000) >> 16;
    tssd->ar = ar & 0x00ff;
    /*
    P DPL 0  1 0 busy 1
    */
    tssd->limithi_AVL_0_0_G = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
    tssd->base_hi = (base & 0xff000000) >> 24;
    return;
}
void task_b_main(void)
{
    struct FIFO8 local_fifo; //全局 从生成的timer中获取
    unsigned char buf[32];
    fifo8_init(&local_fifo,32,buf);
    struct TIMER *switch_timer,*show_timer;
    unsigned char timer_id;
    
    switch_timer = new_timer(TASK_SWITCH_SCLICE,(unsigned char)'t',&local_fifo); // task1
    show_timer = new_timer(SHOW_STH_GAP,(unsigned char)'s',&local_fifo); // task1
    int i = 0;
    int mycount = 0;
    char scount[32];
    extern struct LAYER* backgroundlayer;
    for (;;)
    {
        char s[100];
        io_cli();
        mycount++;
        //  sprintf(scount,"task b : %d",mycount);
        //  debugPrint(scount);
        if (fifo8_status(&local_fifo) == 0)
        { 
            io_sti();
            io_hlt();
        }
        else
        {
            timer_id = fifo8_get(&local_fifo); 
            io_sti();

            if (timer_id == (unsigned char)'t')
            {
                task_switch(0x38);
                switch_timer = new_timer(TASK_SWITCH_SCLICE,(unsigned char)'t',&local_fifo);
                //show_timer = new_timer(SHOW_STH_GAP,(unsigned char)'s',&local_fifo);
                //从这里切回来 并没有一个机会触发s
                //自己负责启动自己的定时器。。
            }else if(timer_id == (unsigned char)'s'){ //有可能在切出去的时候错失这个定时器不过没关系
                sprintf(scount,"task b : %d",mycount);
                putstr_on_layer(backgroundlayer,0,144, MANGO, BABYBLUE,scount,32);
                show_timer = new_timer(SHOW_STH_GAP,(unsigned char)'s',&local_fifo);
            }
        }
        //  io_hlt();
    }
}
