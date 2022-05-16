//for qemu
#include "time.h"
#ifndef _H_TASK
#define _H_TASK
#define TASK_SWITCH_SCLICE 10
#define SHOW_STH_GAP 5
#define CURSOR_COL_SWAP_GAP 10
//最多有10个优先级队列 每个队列中最多有100个任务
#define MAX_TASKS_LV	100
#define MAX_TASKLEVELS	10
struct TSS32
{
    int previousTaskLink;
    int esp0;
    int ss0;
    int esp1;
    int ss1;
    int esp2;
    int ss2;
    int cr3;
    int eip;
    int eflags;

    int eax,ecx,edx,ebx,esp,ebp,esi,edi;
    int es,cs,ss,ds,fs,gs;

    int ldtSegmentSelector;
    int ioMapBaseAddress;
};
//can only be placed in gdt
/*
ar:
高-》低
P DPL(2) 0 1 0 B 1
最高特权级未运行任务应该是
1 00     0 1 0 0 1
*/
struct TSSDescriptor
{
    short limit_low;
    short base_low;
    char base_mid;
    char ar,limithi_AVL_0_0_G;
    char base_hi;
};
struct TaskGateDescriptor
{
    /* data */
    int TSSSegmentSelector;//高16位
    int type_dpl_p;
};


/*task controller*/
#define AR_TSS32    0x89 //非busy
#define MAX_TASKS		1000	/* 最多的任务数量 */
#define TASK_GDT0		7		/* 从gdt的第几项开始是TSSD */

struct TASK {
	int sel, flags; /*selector state_flag */
    int level,priority;//增加优先级：task_switch_slice 
	struct TSS32 tss;
    struct LAYER* bind_layer;
    struct FIFO32 *keyfifo;
};
struct TASKLEVEL {
	int running; //该优先级队列中正在运行的任务数量
	int now; //正在运行的任务 用哪个下标访问
	struct TASK *tasks[MAX_TASKS_LV];
};

struct TASKCTL {
	//int running; /* 就绪任务数 */
    //不在统计所有的任务数量
	int now_lv; /*正在运行的任务所在的优先级队列的下标*/
	char lv_change; //调度后就检查下次是否要切换level
	struct TASKLEVEL level[MAX_TASKLEVELS];
	struct TASK tasks0[MAX_TASKS];
};

void set_TSSDescriptor(struct TSSDescriptor *tssd, int limit, int base, int ar);
void load_tr(int offset);
void task_b_main(struct TASK *thistask);
void schedule(int eip,int cs);
void task_switch(void);
int get_tr(void);

extern struct TIMER *task_timer;
/*初始化任务管理机制 并为当前任务（交互循环）注册为0号任务 并返回地址*/
struct TASK *task_init();
/*在task0中找一个位置 并填充TSS */
struct TASK *task_alloc(void);
/*本质是多了一个就绪的任务 修改running等属性*/
void task_run(struct TASK *task, int level, int priority);
/*内部调用jmp*/
void task_switch(void);
/*和run相对 任务遇到了阻塞事件 将自己从running队列里拿出来 并调用task_switch*/
void task_sleep(struct TASK *task);

//now任务不再通过task_controller直接获取 而是
struct TASK *task_now(void);
//前置条件：task已经填入level
void task_add(struct TASK *task);
void task_remove(struct TASK *task);//从相应的就绪队列中删除 仍然存在于ctl的记录中
void task_switchsub(void);
 #endif