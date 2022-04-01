#define TASK_SWITCH_SCLICE 20
#define SHOW_STH_GAP 100
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
void set_TSSDescriptor(struct TSSDescriptor *tssd, int limit, int base, int ar);
void load_tr(int offset);
void task_b_main(void);
void task_switch(int cs);