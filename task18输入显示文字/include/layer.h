/*layer manager*/
#include "memman.h"
#include "graphic.h"
#include "stdio.h"
#ifndef _L_H
#define _L_H
#define MAX_LAYER_NUM 256
#define LAYER_USE 1
/*size:4+16+12 = 32*/
struct LAYER{
    //value 部分
    unsigned char * buf;//图层的像素颜色
    int bxsize,bysize,vx0,vy0;//b-block v - vram
    int col_inv,height,flags;//
};
/*size:4+4+256*32 + 256*4  = 8KB+1KB+8B*/ //memman需要手动分配 有了memman之后利用其就可！
struct LAYERMAN
{
    struct BOOTINFO * binfo;
    int top;//最上层图层的高度？
    struct LAYER layerset[MAX_LAYER_NUM];
    struct LAYER *ordered_layers[MAX_LAYER_NUM];
};
struct LAYERMAN *layman;
/*
memman alloc
*/
struct LAYERMAN* layerman_init();
struct LAYER *layer_register(struct LAYERMAN *layman);
void layer_setbuf(struct LAYER *layer, unsigned char *buf, int xsize, int ysize, int col_inv);
void layer_updown(struct LAYERMAN *layman, struct LAYER *layer, int height);
void layers_refresh(struct LAYERMAN *layman);
void layer_slide(struct LAYERMAN *layman, struct LAYER *layer, int vx0, int vy0);
void layer_free(struct LAYERMAN *layman, struct LAYER *layer);

struct LAYER* layer_screen(int x, int y);
struct LAYER* layer_mouse(int x,int y);
void layers_refresh_v2(struct LAYERMAN *layman, int vx0, int vy0, int vx1, int vy1);
void layers_refresh_v3(struct LAYERMAN *layman,int h0, int vx0, int vy0, int vx1, int vy1);
struct LAYER* layer_window(int posix,int posiy,int sizex,int sizey);
struct LAYER* layer_time(void);
void make_textbox8(struct LAYER* layer, int x0, int y0, int x1, int y1, int c);
void putstr_on_layer(struct LAYER *layer, int x, int y, int c, int b, char *s,int strlen);
void putcursor_on_layer(struct LAYER *layer, int x, int y, int c);
#endif