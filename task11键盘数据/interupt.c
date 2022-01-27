#include "interupt.h"
#include "time.h"
#include "name.h"
#include <stdio.h>
//中断描述符结构
//初始化中断描述符表
void init_idt(void)
{
	//struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
    //0x17E00 + 0x800 = 0x18600 的地方有256个8字节的中断描述符
	//struct INTERUPT_GATE_DESCRIPTOR *idt = (struct INTERUPT_GATE_DESCRIPTOR*) IDT;
	extern struct INTERUPT_GATE_DESCRIPTOR *idt;
	int i;
	/* IDT */
	for (i = 0; i < 256; i++) {
	//	set_intgatedesc((struct INTERUPT_GATE_DESCRIPTOR*)(idt + i), exception,0x30,0x8e00);//idt+i 地址是idt+i*(idt指向的包含物的单位大小即8)
		set_intgatedesc(idt+i,(int)exception,0x30,0x8e00);//idt+i 地址是idt+i*(idt指向的包含物的单位大小即8)
	}
	load_idtr(0x7ff, 0x17e00);//偏移量 起始地址 idtr就由这两部分组成（而段寄存器是不是默认ds啦）
	
	return;
}

//设置中断门描述符 中断表位置 第几项 中断程序位于的段的段选择子（16位）和程序在的段内偏移
//还有一些type之类的规定值在函数内部写死
void set_intgatedesc(struct INTERUPT_GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	//gd->dw_count     = 0x00;//高字0~7位 中断是10001110
	gd->dw_count     = ar & 0x00ff;//高字0~7位 中断是10001110
	gd->access_right = (ar & 0xff00) >> 8;//高字8~16位 全0
	//gd->access_right = 0x8e;//高字8~16位 全0
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
void exceptionprint(void){
//	extern struct BOOTINFO *binfo;
//	putfont8_asc(binfo->vram, binfo->scrnx, 8, 16, ROSE,(unsigned char*)"an exception!");
}
void showmouse(void){
	extern struct BOOTINFO *binfo;
	putfont8_asc(binfo->vram, binfo->scrnx, 8, 24, ROSE,(unsigned char*)"mouse!");	
}


/*
	int021 8259主片IRQ1 键盘中断
	设备结束标志0x61
	结束汇报端口0x20
*/
int debug_position = 1;
void savekbdata(void){
	extern struct BOOTINFO *binfo;
	//savedata
	int data;
	data = io_in8(0x60);
	//telldone
	io_out8(0x20,0x61);
	io_out8(0xa0,0x61);
	char *s;
	sprintf(s,"%d",data);
	extern int debug_position;
	putfont8_asc(binfo->vram, binfo->scrnx, 8,16*debug_position , ROSE,(unsigned char*)s);	
	debug_position++;
}
