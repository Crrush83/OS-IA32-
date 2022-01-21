#include "time.h"
#include "name.h"
#include "dsctb.h"
void showint0x70(void){
	extern struct BOOTINFO *binfo;
	putfont8_asc(binfo->vram, binfo->scrnx, 8, 8, ROSE, (unsigned char*)"cpu recieve int 0x70!");
	return;
}
void install_clock_int(void){
struct INTERUPT_GATE_DESCRIPTOR *idt = (struct INTERUPT_GATE_DESCRIPTOR *)IDT;
    set_intgatedesc(idt+0x70,(int)int0x70,0x30,0x8e00); 
}