#include "keyboardmouse.h"
#include "fifo.h"
#include "io.h"
#include "graphic.h"
#include <stdio.h>
#include "time.h"

extern struct FIFO8 *keyfifo;
extern struct FIFO8 *mousefifo;

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

void wait_KBC_sendready(void)
{
	for (;;)
	{
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0)
		{
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

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

void enable_mouse(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE); //根据索引端口决定是给kbc的还是连在kbc上的mouse的
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;
}

//=(struct FIFO8 *)0x0ffc;//未初始化数据在这个文件就可 其他文件就不可？
int dbg = 0;
void savemousedata(void)
{
	//extern struct BOOTINFO *binfo;
	extern struct FIFO8 *mousefifo;
	int r = io_in8(0x60);
	fifo8_put(mousefifo, r);
	return;
}

/*
	int021 8259主片IRQ1 键盘中断
	设备结束标志0x61
	结束汇报端口0x20
*/
//初始化遇到：interrupt(): gate descriptor is not valid sys seg
//(vector=0x08)

void savekbdata(void)
{
	//getdata
	unsigned char data;
	data = io_in8(0x60);
	fifo8_put(keyfifo, data);

	// char p[20];
	// sprintf(p, "%d", data);
	// extern struct BOOTINFO *binfo;
	// box_fill8(binfo->vram, binfo->scrnx, WHITE,8, 180, 31,195 );
	// putfont8_asc(binfo->vram, binfo->scrnx, 8, 180, ROSE, (unsigned char *)p);

	//telldone
	// io_out8(0x20, 0x20); //0x61
	// io_out8(0xa0, 0x20); //键盘不需要通知从片吧
	//原子性怎么保证哇？
	return;
}

/*处理鼠标phase
struct MOUSE_DEC {
	unsigned char buf[3], phase;
};
*/
void init_mouse_decode(struct  MOUSE_DEC *mdec)
{
	mdec->phase = 0;	
	mdec->buf[0] = 0x00;
	mdec->buf[0] = 0x00;
	mdec->buf[0] = 0x00;
}
/*
save data 
if ALL recieve return 1
*/
int mouse_decode(struct MOUSE_DEC *mdec,unsigned char dat){
if (mdec->phase == 0) {
		//初始化返回0xfa
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		
		if ((dat & 0xc8) == 0x08) {
			//0x1100 1000 7 6 3位检查
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;//00000111 第一个字节的低三位是按键信息
		mdec->x = mdec->buf[1];//后两个字节是鼠标所在的位置
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) { //0001 0000 第四位检验x 第五位检验y
			mdec->x |= 0xffffff00; //高位为什么置1
	}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y; //鼠标向上滑 y本来该- 但鼠标给出的是正值
		return 1; //把无符号数当作有符号数理解
	}

		 return -1;
}

 void move_mouse(struct BOOTINFO *binfo,struct MOUSE_DEC *mdec, int *mousexp,int *mouseyp){
	 *mousexp = (*mousexp)+mdec->x;
	 *mouseyp = (*mouseyp)+mdec->y;
	 
	 if(*mousexp < 0){
		 *mousexp = 0;
	 }
	 if(*mousexp > binfo->scrnx -1){
		 *mousexp = binfo->scrnx -1;
	 }
	 if(*mouseyp < 0){
		 *mouseyp = 0;
	 }
	 if(*mouseyp > binfo->scrny -1){
		 *mouseyp = binfo->scrny -1;
	 }
 }

	unsigned char keymap[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '@', '[', 0,   0,   'a', 's',
		'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', ':', 0,   0,   ']', 'z', 'x', 'c', 'v',
		'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
	//退格：14 回车：28

	void swap_cursor_color(unsigned char tid){
		extern int cursor_x0,cursor_x,cursor_y0 ,cursor_y,cursor_c;//文字框起始于4 20
		   if(tid == 'b'){//black cursor
                new_timer(2,'w');
                cursor_c = BLACK;  
             }
          if(tid == 'w'){
               new_timer(2,'b');
               cursor_c = WHITE;
             }      
	}