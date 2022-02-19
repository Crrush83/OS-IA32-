#include "multitask.h"
struct TSS32 tss_a,tss_b;

void set_TSSDescriptor(struct TSSDescriptor *tssd, int limit, int base, int ar)
{
	tssd->limit_low   = limit & 0x00ff;
	tssd->base_low    = base & 0x00ff;
	tssd->base_mid = (base >> 16) & 0x00ff;
    tssd->ar = ar & 0xff;
    /*
    P DPL 0  1 0 busy 1
    */
   tssd->limithi_AVL_0_0_G = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
   tssd->base_hi = (base >> 24) & 0x00ff;
    return;
}
void task_b_main(void){
    for(;;){
        io_hlt();
    }
}

