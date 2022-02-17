#include "time.h"
#include "io.h"
#include "interupt.h"
#include "graphic.h"
#include "layer.h"
#include <stdio.h>
#include "fifo.h"
#include "list.h"
#include "memman.h"
void showtime(void){
    // char line[20];
    // int count = *(int*)(0x1010);
    // sprintf(line,"%d",count);
    // debugPrint(line);
	extern struct LAYER *timelayer;
    extern struct LAYERMAN *layman;
    unsigned char *s = (unsigned char *)0xffc;
    //先刷背景再字体没错啊 怎么重影？
    box_fill8(timelayer->buf,64,MOON,0,0,63,15);
 	putfont8_asc(timelayer->buf,64,0,0, BLACK, s);
    layers_refresh_v3(layman,0,timelayer->vx0,timelayer->vy0,
    timelayer->vx0+63,timelayer->vy0+15);
	return;
}
void install_clock_int(void){
    extern struct INTERUPT_GATE_DESCRIPTOR *idt;
    set_intgatedesc(idt+0x70,(int)int0x70,0x30,0x8e00); 
//原来的类型就是IDT 所有加法隐含着乘法
}
struct FIFO8 *timerfifo;
struct TIMERMAN * timerman_init(void){//
    struct TIMERMAN *t = (struct TIMERMAN *)memman_alloc(24);
    t->nexttimeout = (unsigned int)0xffffffff;//这句没成功呀 不完整类型？？
    t->past = 0;
    t->dumbhead.next = &(t->dumbtail);
    t->dumbtail.prev = &(t->dumbhead);
    return t;
}
//有新的定时器插入时
struct TIMER* new_timer(unsigned int timeout,unsigned int id){
     extern struct TIMERMAN *timerman;
     struct TIMER *timer = (struct TIMER *)memman_alloc(24);
     //这个node嵌入到timer里 在内存中相邻
     if(timeout < 1) timeout = 1;
     timer->timeout = timeout;
     timer->id = id;
     timer->presize = 16;
    //timer->node = new_node();//freetimer的时候先free 又要根据list寻找前面的timer结构
     //所以不是以node *的形式存储 而是以node本身 timer->node这样是node域的内容。。
     //传进timer->node 就是0
     //传进&(timer->node) 就是timer+16
     io_cli();
     //禁止时钟中断 
     struct list_node *node = timerman->dumbhead.next;
     struct TIMER *t;
     //均未超nexttimeout
     timerman->nexttimeout = timeout;
     timerman->past = 0;//集体对齐时刻
     for(;node!=&(timerman->dumbtail);node = node->next){
        t = (struct TIMER *)container_of(node);
        t->timeout -= timerman->past;
        if(t->timeout < timeout){
            timerman->nexttimeout = t->timeout;
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
    extern struct TIMERMAN *timerman;
    memman_free((unsigned int)timer,sizeof(struct TIMER));

}
unsigned int sectimeout(struct TIMERMAN *timerman){
    //timerman->list 就是dump节点 
    //.next->next是第二个
    struct TIMER *t = container_of((timerman->dumbhead).next);
    return t->timeout;
}
void updatetimer(void){
    extern struct TIMERMAN *timerman;
    struct list_node *node = timerman->dumbhead.next;
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
    for(;node!=&(timerman->dumbtail);node = node->next){
        t = container_of(node);
        t->timeout -= timerman->past;
        if(t->timeout == 0){
            fifo8_put(timerfifo,t->id);
            free_timer(t);
            continue;
        }
        if(t->timeout > 0){
            timerman->nexttimeout = t->timeout;
            timerman->past = 0;
            return;
        }
    }
    //在计时器全部超时的时候 
}