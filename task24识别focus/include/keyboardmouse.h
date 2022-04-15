#include "io.h"
#include "graphic.h"

void wait_KBC_sendready(void);
void init_keyboard(void);
void enable_mouse(void);
void showmouse(void);
void savekbdata(void);
void handlemousedata(void);
struct MOUSE_DEC {
	unsigned char buf[3],phase;
	int x,y,btn;
};
void init_mouse_decode(struct  MOUSE_DEC *mdec);
//int mouse_decode(struct MOUSE_DEC *mdec,char data);
int mouse_decode(struct MOUSE_DEC *mdec,unsigned char data);
//一定要声明成unsigned
void move_mouse(struct BOOTINFO *binfo,struct MOUSE_DEC *mdec, int *mousexp,int *mouseyp);
void swap_cursor_color();