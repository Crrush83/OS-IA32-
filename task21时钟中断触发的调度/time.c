#include "time.h"
#include "io.h"
#include "interupt.h"
#include "graphic.h"
#include "layer.h"
#include <stdio.h>
#include "fifo.h"
#include "list.h"
#include "memman.h"
#include "multitask.h"
void showtime(void){
    // char line[20];
    // int count = *(int*)(0x1010);
    // sprintf(line,"%d",count);
    // debugPrint(line);
	extern struct LAYER *timelayer;
    unsigned char *s = (unsigned char *)0xffc;
    //先刷背景再字体没错啊 怎么重影？
    box_fill8(timelayer->buf,64,MOON,0,0,63,15);
 	putfont8_asc(timelayer->buf,64,0,0, BLACK, s);
    layers_refresh_v3(0,timelayer->vx0,timelayer->vy0,
    timelayer->vx0+63,timelayer->vy0+15);
	return;
}
void install_clock_int(void){
    extern struct INTERUPT_GATE_DESCRIPTOR *idt;
    set_intgatedesc(idt+0x70,(int)int0x70,0x30,0x8e00); 
//原来的类型就是IDT 所有加法隐含着乘法
}
struct FIFO8 *timerfifo;//timer的全局通知器！！！
//重新设计一个缓友好好的
struct TIMERMAN * timerman_init(void){//
    struct TIMERMAN *t = (struct TIMERMAN *)memman_alloc(24);
    t->nexttimeout = (unsigned int)0xffffffff;//这句没成功呀 不完整类型？？
    t->past = 0;
    t->dumbhead.next = &(t->dumbtail);
    t->dumbtail.prev = &(t->dumbhead);
    return t;
}
//有新的定时器插入时
//fifo域也要填
struct TIMER* new_timer(unsigned int timeout,unsigned int id,struct FIFO8 *bind_fifo){
     extern struct TIMERMAN *timerman;
     struct TIMER *timer = (struct TIMER *)memman_alloc(24);
     //这个node嵌入到timer里 在内存中相邻
     if(timeout < 1) timeout = 1;
     timer->fifo = bind_fifo;
     timer->timeout = timeout;
     timer->id = id;
     timer->presize = 16;
    //timer->node = new_node();//freetimer的时候先free 又要根据list寻找前面的timer结构
     //所以不是以node *的形式存储 而是以node本身 timer->node这样是node域的内容。。
     //传进timer->node 就是0
     //传进&(timer->node) 就是timer+16
     io_cli();
     //禁止时钟中断 
     struct list_node *node; 
     struct TIMER *t;
     //先集中更新在则位置插入哇 刚插入的一定是没有超时的。。
    for(node = timerman->dumbhead.next;node!=&(timerman->dumbtail);node = node->next){
        t = container_of(node);
        t->timeout -= timerman->past;
    }
    timerman->past = 0;//集体对齐时刻
    timerman->nexttimeout = timeout;//假设新加入的是最快超时的 

     for(node = timerman->dumbhead.next;node!=&(timerman->dumbtail);node = node->next){
        t = (struct TIMER *)container_of(node);
        if(t->timeout < timerman->nexttimeout){//以前是写的  timeout 都行
            timerman->nexttimeout = t->timeout;
            break;
        }else{
           // timerman->nexttimeout = timeout;//也不用往后找了
           break;
        }
    }
     list_insert_inorderbyuint(&(timerman->dumbhead),&(timerman->dumbtail)
     ,&(timer->node),0);
     io_sti();
     return timer;
 }
void free_timer(struct TIMER* timer){
   // struct list_node  free = timer->node;
    struct list_node* next = timer->node.next;
    struct list_node* prev = timer->node.prev;
    prev->next = next;
    next->prev = prev;
    //extern struct TIMERMAN *timerman;
    memman_free((unsigned int)timer,sizeof(struct TIMER));

}
unsigned int sectimeout(struct TIMERMAN *timerman){
    //timerman->list 就是dump节点 
    //.next->next是第二个
    struct TIMER *t = container_of((timerman->dumbhead).next);
    return t->timeout;
}
int clock = 0;
//char clk[32];
struct TIMER *task_switch_timer;
//after init timerman, call new_timer() set an id, a timeout, and a null buf
//then insert this timer into timer list
//when find a timeout-timer, check if it is the task_switch_timer
//ts_flag = true
//when all the update done, switch task!(in the new task may set allow-interupt = true)
void updatetimer(void){  
    // clock++;
    // char clk;
    // sprintf(clk,"%d",clock);
    // debugPrint(clk);
    int ts_flag = 0;
    extern struct TIMERMAN *timerman;
    struct list_node *node;
    struct TIMER * t;
    if(timerman->dumbhead.next == &(timerman->dumbtail)){
        //无计时器
        return;
    }
    timerman->nexttimeout --;
    timerman->past++;
    if(timerman->nexttimeout > 0){
        return;
    }
    //有计时器超时？nexttimeout == 0 past:所有计时器应该减去的
    for(node = timerman->dumbhead.next;node!=&(timerman->dumbtail);){
        t = container_of(node);
        t->timeout -= timerman->past; //t->timeout == 0也不要break呀 我们要在这里集中更新
        node = node->next;
        //在有可能删除之前更新node
        if(t->timeout == 0){//集中更新
            //本来就是关中断的 先整体更新再解决nexttimeout
            if(t->id == (unsigned char)'t'){
               // debugPrint("task_switch timeout");//
                ts_flag++;
            }else{
            fifo8_put(t->fifo,t->id);//利用全局定时器通知缓冲区通知到时事件
            }
            free_timer(t);//free后next肯定出错哇。。 所以改变取下一个的位置。。
            continue;
        }
        if(t->timeout > 0){
           continue;//一进入循环就更新了呀
        }
    }
    if(timerman->dumbhead.next!=&(timerman->dumbtail)){
        struct TIMER *first = container_of(timerman->dumbhead.next);
        timerman->nexttimeout = first -> timeout;
        timerman->past = 0;
    }else{
        timerman->nexttimeout = 0;
        timerman->past = 0;
    }
    extern int task_switch_flag;
    if(ts_flag > 0){
        task_switch_flag = 1;
    }else{
        task_switch_flag = 0;
    }
    task_switch();
    //在计时器全部超时的时候 
}