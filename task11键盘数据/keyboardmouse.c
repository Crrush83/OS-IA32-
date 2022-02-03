#include "keyboardmouse.h"
#include "fifo.h"
#include "io.h"
#include <stdio.h>
struct FIFO8 keyinfos;
struct FIFO8 * keyinfo = &keyinfos; 

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);//根据索引端口决定是给kbc的还是连在kbc上的mouse的
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return; 
}


//=(struct FIFO8 *)0x0ffc;//未初始化数据在这个文件就可 其他文件就不可？
void showmouse(void){
	extern struct BOOTINFO *binfo;
	putfont8_asc(binfo->vram, binfo->scrnx, 8, 24, ROSE,(unsigned char*)"mouse!");	
}

/*
	int021 8259主片IRQ1 键盘中断
	设备结束标志0x61
	结束汇报端口0x20
*/
//初始化遇到：interrupt(): gate descriptor is not valid sys seg
 //(vector=0x08)

void savekbdata(void){
	//getdata
	unsigned char data;
	data = io_in8(0x60);
	extern struct FIFO8 * keyinfo;
    int r = fifo8_put(keyinfo,data);

	char p[20];
	sprintf(p,"%d",r);
	extern struct BOOTINFO *binfo;
	putfont8_asc(binfo->vram, binfo->scrnx, 8, 180, ROSE,(unsigned char*)p);	
	
	//telldone
	io_out8(0x20,0x20);//0x61
	io_out8(0xa0,0x20); //键盘不需要通知从片吧
	//原子性怎么保证哇？
	return;
}
