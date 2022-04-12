#include "multitask.h"
#include "time.h"
#include "fifo.h"
#include "io.h"
#include "graphic.h"
#include "stdio.h"
#include "layer.h"
#include "memman.h"
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
    sendEOI();
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

/*添加task controller*/
struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init()
{
	int i;
	struct TASK *task;
	struct TSSDescriptor *gdt = (struct TSSDescriptor *) ADR_GDT;
	taskctl = (struct TASKCTL *) memman_alloc_4k(sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		set_TSSDescriptor(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2; /*正在运行状态*/
	taskctl->running = 1; //就绪个数：1
	taskctl->now = 0;
	taskctl->tasks[0] = task; //当前运行任务：0号
	load_tr(task->sel);
	task_timer = new_timer(TASK_SWITCH_SCLICE,'t',NULL);
	return task;
}

struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; /*就绪状态*/
		//	task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; 
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtSegmentSelector = 0;
			task->tss.ioMapBaseAddress = 0x40000000;
			return task;
		}
	}
	return 0; 
}

void task_run(struct TASK *task)
{
	task->flags = 2; 
	taskctl->tasks[taskctl->running] = task;
	taskctl->running++;
	return;
}

void task_switch(void)
{

    new_timer(TASK_SWITCH_SCLICE,'t',NULL);
	if (taskctl->running >= 2) {
		taskctl->now++;
		if (taskctl->now == taskctl->running) {
			taskctl->now = 0;
		}
		schedule(0, taskctl->tasks[taskctl->now]->sel);
	}
	return;
}

void task_sleep(struct TASK *task)
{
	int i;
	char ts = 0;
	if (task->flags == 2) {		
		if (task == taskctl->tasks[taskctl->now]) {
			ts = 1;
		}

		for (i = 0; i < taskctl->running; i++) {
			if (taskctl->tasks[i] == task) {

				break;
			}
		}
		taskctl->running--;
		if (i < taskctl->now) {
			taskctl->now--; 
		}

		for (; i < taskctl->running; i++) {
			taskctl->tasks[i] = taskctl->tasks[i + 1];
		}
		task->flags = 1; 
		if (ts != 0) {

			if (taskctl->now >= taskctl->running) {
		
				taskctl->now = 0;
			}
			schedule(0, taskctl->tasks[taskctl->now]->sel);
		}
	}
	return;
}

