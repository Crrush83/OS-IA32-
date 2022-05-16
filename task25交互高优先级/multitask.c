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
void task_b_main(struct TASK *thistask)
{
    sendEOI();//EOI为什么只需要一次呢？
    struct FIFO32 local_fifo; //全局 从生成的timer中获取
    int buf[32];
    fifo32_init(&local_fifo,32,buf);
    new_timer(SHOW_STH_GAP,(unsigned char)'s',&local_fifo); // task1 这个定时器有问题？
    int timer_id;
    int i = 0;
    int mycount = 0;
    char scount[32];
    extern struct LAYER* backgroundlayer;

	//想在刚切回来的时候发送EOI 但是切出去的时机无法预测？
    for (;;)
    {
		sendEOI();
        mycount++;
        char s[100];
        io_cli();
        if (fifo32_status(&local_fifo) == 0)
        { 
           io_sti();//只调用它会卡住吗？
        }
        else
        {
          timer_id = fifo32_get(&local_fifo); 
            io_sti();
            if(timer_id == (unsigned char)'s'){ //有可能在切出去的时候错失这个定时器不过没关系
                sprintf(scount,"%d",mycount);
                putstr_on_layer(thistask->bind_layer,24,
				20, MANGO, MOON,scount,32);
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

	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}


	task = task_alloc();
	task->flags = 2;	
	task->priority = TASK_SWITCH_SCLICE; 
	task->level = 0;	
	task_add(task);
	task_switchsub();	//决定下一次调度的now lv
	load_tr(task->sel);
	task_timer = new_timer(task->priority,'t',NULL);
	//增加优先级后 特定的slice
	
	return task;
}
/*找到第一个flags为0的任务 分配之 返回后填写属性 再加入到就绪队列中*/
struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->keyfifo = NULL;//
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
/*仅仅意味着加入到就绪队列中 不代表切换哦*/
void task_run(struct TASK *task, int level, int priority)
{

	if (level < 0) {
		level = task->level; 
	}
	if (priority > 0) {
		task->priority = priority;
	}

	if (task->flags == 2 && task->level != level) {
		task_remove(task); 
	}
	if (task->flags != 2) {
		task->level = level;
		task_add(task);
	}

	taskctl->lv_change = 1; 
	return;
}
void task_switch(void)
{

	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	//轮换当前队列
	tl->now++;
	if (tl->now == tl->running) {
		tl->now = 0;
	}
	//是否需要重新考虑优先级变化
	if (taskctl->lv_change != 0) {
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	new_timer(new_task->priority,'t',NULL); 
	if (new_task != now_task) {
		schedule(0, new_task->sel);
	}
	return;

}
/*把自己从就绪队列中拿出来*/
/*void task_sleep(struct TASK *task)
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
*/

//优先级队列
struct TASK *task_now(void){
    struct TASKLEVEL *tl = &taskctl -> level[taskctl -> now_lv];
    return tl->tasks[tl->now];
}
void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;
	tl->running++;
	task->flags = 2; //默认是就绪的任务
	return;
}
//代替sleep
void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	//获取该任务存在于的队列
	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			break;
		}
	}
//tl队列中下标为i的任务
	tl->running--;
	//running会是0吗？可以是0 这个队列不被调度哇
	if (i < tl->now) {
		tl->now--; //该队列的运行总数递减
	}
	if (tl->now >= tl->running) {
		tl->now = 0;
	}
	task->flags = 1; 
//running是个数 now却从0开始 那么拿掉i后 任务有效下标是0~running(旧)-2
//新running仍然只比有效task下标大1

	//从i开始填充
	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}

	return;
}
void task_switchsub(void)
{
	int i;
	//寻找最上层的level
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break;
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}