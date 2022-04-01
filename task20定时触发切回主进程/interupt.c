#include "interupt.h"
#include "time.h"
#include "io.h"
#include "fifo.h"
#include <stdio.h>
//补齐段表
void init_gdt(void){
	//0x00007e00 + 56(0x38) = 0x7e;38
	
        //  write_mem32(0x7e00,0x00);
        //  write_mem32(0x7e04,0x00);
        //  write_mem32(0x7e08,0x7c0001ff );
        //  write_mem32(0x7e0c,0x00409800 );
        //  write_mem32(0x7e10,0x8000ffff );
        //  write_mem32(0x7e14,0x0040920b);
        //  write_mem32(0x7e18,0x00000000);
        //  write_mem32(0x7e20,0x0000ffff );
        //  write_mem32(0x7e24,0x00419810);
        //  write_mem32(0x7e28,0x00001fff);
        //  write_mem32(0x7e2c,0x00c09200);
        //  write_mem32(0x7e30,0x00001fff);
        //  write_mem32(0x7e34,0x00c09a00);
	// int addr = 0x7e38;

	// for(;addr < 0x17dff;addr += 0x8){
	// 	write_mem32(addr,0x00001fff);
	// 	write_mem32(addr+0x4,0x00c09200);
	// }
	//load_gdtr();
		// mov dword [bx+0x28],0x00001fff
		// mov dword [bx+0x2c],0x00c09200
}
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
	//也有一些error code不需要pop
	set_intgatedesc(idt+8,(int)dealerrorcode,0x30,0x8e00);
	set_intgatedesc(idt+10,(int)dealerrorcode,0x30,0x8e00);
	set_intgatedesc(idt+11,(int)dealerrorcode,0x30,0x8e00);
	set_intgatedesc(idt+12,(int)dealerrorcode,0x30,0x8e00);
	set_intgatedesc(idt+13,(int)dealerrorcode,0x30,0x8e00);
	set_intgatedesc(idt+14,(int)dealerrorcode,0x30,0x8e00);
	set_intgatedesc(idt+17,(int)dealerrorcode,0x30,0x8e00);
	//处理errorcode的
	
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
