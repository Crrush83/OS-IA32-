#include "time.h"
#include "io.h"
#include "interupt.h"
#include "graphic.h"
#include "layer.h"
#include <stdio.h>
void showtime(void){
    char line[20];
    int count = *(int*)(0x1010);
    sprintf(line,"%d",count);
    debugPrint(line);
	extern struct LAYER *timelayer;
    extern struct LAYERMAN *layman;
    unsigned char *s = (unsigned char *)0xffc;
    //先刷背景再字体没错啊 怎么重影？
    box_fill8(timelayer->buf,64,MOON,0,0,63,15);
 	putfont8_asc(timelayer->buf,64,0,0, BLACK, s);
    layers_refresh_v3(layman,0,timelayer->vx0,timelayer->vy0,
    timelayer->vx0+63,timelayer->vy0+15);
	return;
}
void install_clock_int(void){
    extern struct INTERUPT_GATE_DESCRIPTOR *idt;
    set_intgatedesc(idt+0x70,(int)int0x70,0x30,0x8e00); 
//原来的类型就是IDT 所有加法隐含着乘法
}