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
    struct FIFO8 *notice_buffer; //全局 从生成的timer中获取
    struct TIMER *switch_timer;
    unsigned char timer_id;
    switch_timer = new_timer(20,'t'); // task1
    notice_buffer = switch_timer->fifo;
    int i = 0;
    for (;;)
    {
        char s[100];
        io_cli();
        if (fifo8_status(notice_buffer) == 0)
        {
            io_sti();
            io_hlt();
        }
        else
        {
            timer_id = fifo8_get(notice_buffer); //那么这里拿到的应该是49啊？为什么拿到255呢？
            io_sti();

            if (timer_id == 't')
            {
                task_switch(0x38);
            }
        }
        //  io_hlt();
    }
}
