#include "interupt.h"
//中断描述符结构
//初始化中断描述符表
void init_idt(void)
{
	//struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
    //0x17E00 + 0x800 = 0x18600 的地方有256个8字节的中断描述符
	struct INTERUPT_GATE_DESCRIPTOR *idt = (struct INTERUPT_GATE_DESCRIPTOR*) IDT;
	int i;
	/* IDT */
	for (i = 0; i < 256; i++) {
		set_intgatedesc((struct INTERUPT_GATE_DESCRIPTOR*)(idt + i), 0,0,0);//idt+i 地址是idt+i*(idt指向的包含物的单位大小即8)
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
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
