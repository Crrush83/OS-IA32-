#include "list.h"
#include "memman.h"
struct TIMER{
    unsigned int timeout;//62.5ms一次中断 而在qemu中却是250ms左右一次。。
    unsigned char id;//闹钟id
    struct FIFO8 *fifo;//计时器缓冲器 查看到时事件
    unsigned int presize;//4 + 4 + 4 + 4 = 16
    struct list_node node;//8
};
struct TIMERMAN{
    unsigned int nexttimeout;//链表长度：有效计时器个数
    unsigned int past;
    struct list_node dumbhead;//这个里面是tail和head
    struct list_node dumbtail;//这个里面是tail和head
};//4 + 8 + 8
/*
0f0f0f0f
*/
//时钟 4 + 4
void initRTC(void); 
void install_clock_int(void);
void showtime(void);
struct TIMERMAN * timerman_init(void);
struct TIMER* new_timer(unsigned int timeout,unsigned int id);
void free_timer(struct TIMER* timer);
unsigned int sectimeout(struct TIMERMAN *timerman);
void updatetimer(void);