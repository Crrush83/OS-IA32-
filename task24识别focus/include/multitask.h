//for qemu
#include "time.h"
#ifndef _H_TASK
#define _H_TASK
#define TASK_SWITCH_SCLICE 20
#define SHOW_STH_GAP 50
#define CURSOR_COL_SWAP_GAP 20
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
	struct TSS32 tss;
    struct LAYER* bind_layer;
    struct FIFO32 *keyfifo;
};

struct TASKCTL {
	int running; /* 就绪任务数 */
	int now; /*正在运行的任务下标*/
	struct TASK *tasks[MAX_TASKS]; //就绪队列
	struct TASK tasks0[MAX_TASKS]; //注册表
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
void task_run(struct TASK *task);
/*内部调用jmp*/
void task_switch(void);
/*和run相对 任务遇到了阻塞事件 将自己从running队列里拿出来 并调用task_switch*/
void task_sleep(struct TASK *task);
 #endif