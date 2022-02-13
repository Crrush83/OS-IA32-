#include "layer.h"
#include "memman.h"
#include "graphic.h"
#include <stdio.h>
//*layer man*//
struct LAYERMAN* layerman_init(){
	//extern struct MEMMAN *memman;
	extern struct BOOTINFO *binfo;
	struct LAYERMAN *layerman = (struct LAYERMAN *)memman_alloc_4k(sizeof(struct LAYERMAN)); 
	//addr 0 代表错误 因为最低可用是12000
	if(layerman == 0){
		goto err;
	}
	layerman->binfo = binfo;
	layerman->top = -1;
	int i;
	for(i = 0;i < MAX_LAYER_NUM;i++){
		layerman->layerset[i].flags = 0;
	}
	err:
		return layerman;
}

struct LAYER *layer_register(struct LAYERMAN *layman)
{
	struct LAYER *layer;
	int i;
	for (i = 0; i < MAX_LAYER_NUM; i++) {
		if (layman->layerset[i].flags == 0) {
			layer = &(layman->layerset[i]);
			layer->flags = LAYER_USE; 
			layer->height = -1; 
			return layer;
		}
	}
	return 0;	
}

void layer_setbuf(struct LAYER *layer, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	layer->buf = buf;
	layer->bxsize = xsize;
	layer->bysize = ysize;
	layer->col_inv = col_inv;
	return;
}
/*
隐藏->可见 新高度top+1限制 如果新高度 < top 向右移动空出新位置
可见->隐藏 向左移动补齐空位
可见->可见 高->低 [低，高）向右移动
		   低->高 （低，高]向左移动

*/
void layer_updown(struct LAYERMAN *layman, struct LAYER *layer, int height)
{
	int h, old = layer->height; 
	if (height > layman->top + 1) {
		height = layman->top + 1;//没有必要将新图层高度设置得太高 最多成为top即可
	}
	if (height < -1) {
		height = -1;
	}
	layer->height = height;

	
	if (old > height) {	
		if (height >= 0) {

			for (h = old; h > height; h--) {
				layman->ordered_layers[h] = layman->ordered_layers[h - 1];
				layman->ordered_layers[h]->height = h;
			}
			layman->ordered_layers[height] = layer;
		} else {
			//隐藏
			if (layman->top > old) {
				//不是top隐藏 隐藏图层不在ordered里啊？
				for (h = old; h < layman->top; h++) {
					layman->ordered_layers[h] = layman->ordered_layers[h + 1];
					layman->ordered_layers[h]->height = h;
				}
			}
			layman->top--;
		}
	//修改显示	
	//溢出？
	layers_refresh_v3(layman,0,layer->vx0,layer->vy0,layer->vx0 + layer->bxsize,layer->vy0+layer->bysize);
	} else if (old < height) {	
		if (old >= 0) {
//(old,height]左移
			for (h = old; h < height; h++) {
				layman->ordered_layers[h] = layman->ordered_layers[h + 1];
				layman->ordered_layers[h]->height = h;
			}
			layman->ordered_layers[height] = layer;
		} else {	
//空降height [height,top]右移
//old < height 但可能原来的下标更大
//当原来的下标更小的时候适合左移（右移会造成ordered下标空缺）
			for (h = layman->top; h >= height; h--) {
				layman->ordered_layers[h + 1] = layman->ordered_layers[h];
				layman->ordered_layers[h + 1]->height = h + 1;
			}
			layman->ordered_layers[height] = layer;
			layman->top++;
		}
		layers_refresh_v3(layman,height,layer->vx0,layer->vy0,layer->vx0 + layer->bxsize,layer->vy0+layer->bysize);

	}
	return;
}

void layers_refresh(struct LAYERMAN *layman)
{
	int h, bx, by, vx, vy;
 	unsigned char *buf, c, *vram = layman->binfo->vram;
	struct LAYER *layer;
	for (h = 0; h <= layman->top; h++) {
		layer = layman->ordered_layers[h];
		buf = layer->buf;
		for (by = 0; by < layer->bysize; by++) {
			vy = layer->vy0 + by;
			if(vy >= 200) break;
			for (bx = 0; bx < layer->bxsize; bx++) {
				vx = layer->vx0 + bx;
				if(vx >= 320) continue;
				c = buf[by * layer->bxsize + bx];
				if (c != layer->col_inv) {
					vram[vy * layman->binfo->scrnx + vx] = c;
				}
			}
		}
	}
	return;
}
//刷新区域、图层溢出
//vx0:较小（靠左）的在vram中的水平向起点
//bx:在buf中的水平向坐标
void layers_refresh_v2(struct LAYERMAN *layman, int vx0, int vy0, int vx1, int vy1)
{
	    //      char *tmp = "         ";
        //   sprintf(tmp,"x:%d y:%d",vx0,vy0);
        //   debugPrint((unsigned char*)tmp);
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = layman->binfo->vram;
	struct LAYER *layer;
	// if(vx0 < 0) {vx0 = 0;}
	// if(vy0 < 0) {vy0 = 0;}
	//一做坐标修正就出错？
	//if(vx1 > layman->binfo->scrnx - 1) {vx1 = layman->binfo->scrnx - 1;}
	//if(vy1 > layman->binfo->scrny - 1) {vy1 = layman->binfo->scrny - 1;}

		for (h = 0; h <= layman->top; h++) {
		layer = layman->ordered_layers[h];
		buf = layer->buf;
		/* 设置bx0(buf中的水平起点)以匹配vram的水平起点 */
		/*
		0<=bx<=layer->bxsize - 1
		0<=by<=layer->bysize - 1

		同时

		vx0 <= bx+layer->vx0 <= vx1 //+相对起点 还在合法刷新范围内
		bx >= max{vx0 - layer->vx0 , 0}
		bx <= min{vx1 - layer->vx0 , layer->bxsize - 1}
		*/
		int bx0 = vx0 - layer->vx0;
		if (bx0 < 0) { bx0 = 0; }
		int by0 = vy0 - layer->vy0;
		if (by0 < 0) { by0 = 0; }

		int bx1 = vx1 - layer->vx0;
		if (bx1 >= layer->bxsize) { bx1 = layer->bxsize - 1; }
		int by1 = vy1 - layer->vy0;
		if (by1 >= layer->bysize) { by1 = layer->bysize - 1; }

		for (by = by0; by <= by1; by++) {
			vy = layer->vy0 + by;//实际在vram中的纵向坐标
			if(vy >= 200){continue;}
			for (bx = bx0; bx <= bx1; bx++) {
				vx = layer->vx0 + bx;
				if(vx >= layman->binfo->scrnx){continue;}
				c = buf[by * layer->bxsize + bx];//不要超过 16 * 16 -1 小心咯
				if (c != layer->col_inv) {
					vram[vy * layman->binfo->scrnx + vx] = c;
				}
			}
		}
	}
	return;
}


void layers_refresh_v3(struct LAYERMAN *layman,int h0, int vx0, int vy0, int vx1, int vy1)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = layman->binfo->vram;
	struct LAYER *layer;
	for (h = h0; h <= layman->top; h++) {
		layer = layman->ordered_layers[h];
		//ordered里的没有不可见
		buf = layer->buf;
		/* 设置bx0(buf中的水平起点)以匹配vram的水平起点 */
		/*
		0<=bx<=layer->bxsize - 1
		0<=by<=layer->bysize - 1

		同时

		vx0 <= bx+layer->vx0 <= vx1 //+相对起点 还在合法刷新范围内
		bx >= max{vx0 - layer->vx0 , 0}
		bx <= min{vx1 - layer->vx0 , layer->bxsize - 1}
		*/

		int bx0 = vx0 - layer->vx0;
		if (bx0 < 0) { bx0 = 0; }
		int by0 = vy0 - layer->vy0;
		if (by0 < 0) { by0 = 0; }

		int bx1 = vx1 - layer->vx0;
		if (bx1 >= layer->bxsize) { bx1 = layer->bxsize - 1; }
		int by1 = vy1 - layer->vy0;
		if (by1 >= layer->bysize) { by1 = layer->bysize - 1; }

		for (by = by0; by <= by1; by++) {
			vy = layer->vy0 + by;//实际在vram中的纵向坐标
			if(vy >= 200){continue;}
			for (bx = bx0; bx <= bx1; bx++) {
				vx = layer->vx0 + bx;
				if(vx >= 320){continue;}
				c = buf[by * layer->bxsize + bx];//不要超过 16 * 16 -1 小心咯
				if (c != layer->col_inv) {
					vram[vy * layman->binfo->scrnx + vx] = c;
				}
			}
		}
	}
	return;
}

void layer_slide(struct LAYERMAN *layman, struct LAYER *layer, int vx0, int vy0)
{
	

	if (layer->height >= 0) { 
		//ab
		int oldx = layer->vx0,oldy = layer->vy0;
		layer->vx0 = vx0;
	    layer->vy0 = vy0;
        layers_refresh_v3(layman,0,oldx,oldy,
		oldx+layer->bxsize - 1,oldy+layer->bysize -1); 
		layers_refresh_v3(layman,layer->height,vx0,vy0,
		 vx0+ layer->bxsize-1,vy0+layer->bysize -1);
	}else{
	layer->vx0 = vx0;
	layer->vy0 = vy0;}
	return;

}
//高度-1 目的在ordered里空出位置
//同时标记为未使用 在layserset里空出位置
void layer_free(struct LAYERMAN *layman, struct LAYER *layer)
{
	if (layer->height >= 0) {
		layer_updown(layman, layer, -1); 
	}
	layer->flags = 0; 
	return;
}

/*背景图层*/
struct LAYER* layer_screen(int x, int y)
{
	extern struct LAYERMAN *layman;
	//mem space 
	unsigned char * bgbuf = (unsigned char * )memman_alloc_4k(x * y);
	init_screen(bgbuf,x,y);
	struct LAYER * layerbg;
	layerbg = layer_register(layman);
	layer_setbuf(layerbg,(unsigned char *)bgbuf,x,y,16);
	layer_updown(layman,layerbg,0);
	//默认 0，0 位置
	return layerbg;
}
/*鼠标图层*/
struct LAYER* layer_mouse(int x,int y){
	extern struct LAYERMAN *layman;
	unsigned char * mcursor = (unsigned char *)memman_alloc(256);//是全局变量还是堆上变量 应该不影响
  //  extern unsigned char *mcursor;
	struct LAYER * layerms;
	layerms = layer_register(layman);
	layerms->vx0 = 0;
	layerms->vy0 = 0;
	init_mouse_cursor8(mcursor,BABYBLUE);
	layer_setbuf(layerms,(unsigned char *)mcursor,x,y,BABYBLUE);
	layer_updown(layman,layerms,layman->top+1);//always max height
	return layerms;
}
#define CLOSE 0x0
#define CLOSEFOCUS 0x1
#define MINI 0x2
#define MINIFOCUS 0x3
struct LAYER* layer_window(int posix,int posiy,int sizex,int sizey){
	extern struct LAYERMAN *layman;
	unsigned char * window = (unsigned char *)memman_alloc_4k(sizex*sizey);
	struct LAYER *layer;
	layer = layer_register(layman);
	layer->vx0 = posix;
	layer->vy0 = posiy;
	int close_icon_x,close_icon_y,mini_icon_x,mini_icon_y;
	int closex0 = sizex - 16,minix0 = sizex - 32;
	//up
	//left
	//down
	//right
	box_fill8(window,sizex,MOON,1,1,sizex - 2,sizey - 2);
	unsigned char close[224],mini[224];
	init_icon(close,1);
	init_icon(mini,3);
	for(mini_icon_y = 0;mini_icon_y < 14;mini_icon_y++){
		for(mini_icon_x = 0;mini_icon_x < 16;mini_icon_x++)
		{
			window[mini_icon_y * sizex + (minix0) + mini_icon_x] = mini[mini_icon_y*16 + mini_icon_x];
		}
		
	}
	for(close_icon_y = 0;close_icon_y < 14;close_icon_y++){
		for(close_icon_x = 0;close_icon_x < 16;close_icon_x++){
			window[close_icon_y * sizex + closex0 + close_icon_x] = 
			close[close_icon_y * 16 + close_icon_x];
		}
	}
	
	/*
	压边
	void box_fill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
               int x1, int y1) 
	*/

	box_fill8(window,sizex,WHITE,0,0,sizex-1,0);
	box_fill8(window,sizex,WHITE,0,0,0,sizey - 1);
	box_fill8(window,sizex,MOUSE,1,sizey - 1,sizex - 1,sizey - 1);
	box_fill8(window,sizex,MOUSE,sizex - 1,0,sizex - 1,sizey - 1);
	//临时变量 而window的内容保存在堆上
	putfont8_asc(window,sizex,30,30,LAVENDER,"uuu wheres ut");
	layer_setbuf(layer,window,sizex,sizey,-1);//暂时无透明色
	layer_updown(layman,layer,layman->top+1);
	
}

struct LAYER *layer_time(void){
	extern struct LAYERMAN *layman;

	unsigned char * clockbg = (unsigned char *)memman_alloc_4k(1024);//是全局变量还是堆上变量 应该不影响
    box_fill8(clockbg,64,MOON,0,0,63,15);
  //  extern unsigned char *mcursor;
	struct LAYER * layer;
	layer = layer_register(layman);
	layer->vx0 = 247;
	layer->vy0 = 181;
	layer_setbuf(layer,(unsigned char *)clockbg,64,16,MOON);
	layer_updown(layman,layer,layman->top+1);//always max height
	return layer;
}