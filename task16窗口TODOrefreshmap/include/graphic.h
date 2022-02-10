#ifndef _G_H
#define _G_H
struct BOOTINFO{
    char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	unsigned char *vram;
};
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void box_fill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
               int x1, int y1);
void init_screen(unsigned char *vram, int x, int y);          
void putfont8(unsigned char *vram, int xsize, int x, int y, unsigned char color, char *font);     
void init_mouse_cursor8(unsigned char *mouse,unsigned char bc);//拓印一个鼠标 并且背景颜色由bc决定 将鼠标拓印到一个全局鼠标显存内容变量中去
void putblockVRAM(unsigned char *vram, int vxsize, int pxsize,int pysize, int px0, int py0, unsigned char *buf, int bxsize);//bxsize 显示宽度 pxsize 缓存宽度
void putfont8_asc(unsigned char *vram,int xsize,int x,int y,unsigned char c,unsigned char *s);
/*debug*/
void debugPrint(unsigned char * p);

#define BLACK 0x0 // 黑色
#define PINK 0x1
#define ROSE 0x2
#define LAVENDER 0x3
#define BABYBLUE 0x4
#define MINT 0x5
#define MINTBLUE 0x6
#define WHITE 0x7
#define MOON 0x8
#define GRASS 0x9
#define MUSTARD 0xa
#define MANGO 0xb
#define COFFEE 0xc
#define CHINESEINK 0xd
#define ORANGE 0xe
#define MOUSE 0xf


void init_icon(unsigned char* iconbuf,int state);

#endif


