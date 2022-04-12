#include "multitask.h"
#include "time.h"
#include "fifo.h"
#include "io.h"
#include "graphic.h"
#include "stdio.h"
#include "layer.h"
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
    new_timer(SHOW_STH_GAP,(unsigned char)'s',&local_fifo); // task1 这个定时器有问题？
    unsigned char timer_id;
    int i = 0;
    int mycount = 0;
    char scount[32];
    extern struct LAYER* backgroundlayer;
    for (;;)
    {
        sendEOI();//well done!!!太漂亮了！！！
        mycount++;
        char s[100];
        io_cli();
        if (fifo8_status(&local_fifo) == 0)
        { 
           io_sti();//只调用它会卡住吗？
        }
        else
        {
          timer_id = fifo8_get(&local_fifo); 
            io_sti();
            if(timer_id == (unsigned char)'s'){ //有可能在切出去的时候错失这个定时器不过没关系
                sprintf(scount,"taskb show timer : %d",mycount);
                putstr_on_layer(backgroundlayer,0,144, MANGO, BABYBLUE,scount,32);
                new_timer(SHOW_STH_GAP,(unsigned char)'s',&local_fifo);//再次插入后中断就死翘翘？？
           }
        }
          io_hlt();
    }
}
int switchcount = 0;
int current_task;
int task_switch_flag;
//在检查定时器以后设置 所以没有't'定时器超时一定会被设置成0
void task_switch(){
    if(task_switch_flag == 0){

    }else{
        current_task = get_tr();
        //switch过程在中断里 如果过程不是原子的会导致切到正忙的任务？？
        if(current_task == 0x38){
           current_task = 0x40;
         }else{
        current_task = 0x38;    
        }
        new_timer(TASK_SWITCH_SCLICE,'t',NULL);
        //第50次的时候有定时器超时！
    //    debugPrint("schedule");//换成手动。。之前就写两个参数就错 怎么了嘛
        schedule(0,current_task);//中断的最后
     //   debugPrint("back");//换成手动。。之前就写两个参数就错 怎么了嘛
    }

}
