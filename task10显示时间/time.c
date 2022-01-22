#include "time.h"
#include "name.h"
#include "interupt.h"
void showtime(void){
	extern struct BOOTINFO *binfo;
    unsigned char *s = (unsigned char *)0xffc;
    //8*8 16 2^10
    //320 * 200
    //178+22
    /*
    void putblockVRAM(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
    */
char clockbg[1024];
int i;
  for(i = 0;i < 1024;i++){
        clockbg[i] = MOON;
  }
    putblockVRAM(binfo->vram, binfo->scrnx,64,16,247,181,clockbg,64);
 	putfont8_asc(binfo->vram, binfo->scrnx,247,181, BLACK, s);
	return;
}
void install_clock_int(void){
    extern struct INTERUPT_GATE_DESCRIPTOR *idt;
    set_intgatedesc(idt+0x70,(int)int0x70,0x30,0x8e00); 
//原来的类型就是IDT 所有加法隐含着乘法
}