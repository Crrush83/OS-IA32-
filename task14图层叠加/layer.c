#include "layer.h"
#include "memman.h"
#include "graphic.h"
#include <stdio.h>
//*layer man*//
struct LAYERMAN* layerman_init(struct MEMMAN *memman,struct BOOTINFO *binfo){
	struct LAYERMAN *layerman = (struct LAYERMAN *)memman_alloc_4k(memman,sizeof(struct LAYERMAN)); 
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
	layers_refresh(layman);
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
		layers_refresh(layman);
	}
	return;
}

void layers_refresh(struct LAYERMAN *layman)
{
	int h, bx, by, vx, vy;
 char *buf, c, *vram = (char *)layman->binfo->vram;
 char * line;
 sprintf(line,"%X",vram);
 debugPrint(line);
	struct LAYER *layer;
	for (h = 0; h <= layman->top; h++) {
		layer = layman->ordered_layers[h];
		buf = layer->buf;
		for (by = 0; by < layer->bysize; by++) {
			vy = layer->vy0 + by;
			for (bx = 0; bx < layer->bxsize; bx++) {
				vx = layer->vx0 + bx;
				c = buf[by * layer->bxsize + bx];
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
	layer->vx0 = vx0;
	layer->vy0 = vy0;
	if (layer->height >= 0) { 
		layers_refresh(layman); 
	}
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
	extern struct MEMMAN * memman;
	//mem space 
	char * bgbuf = (char * )memman_alloc_4k(memman,x * y);
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
	extern struct MEMMAN * memman;
	extern char * mcursor;
	struct LAYER * layerms;
	layerms = layer_register(layman);
	layerms->vx0 = 151;
	layerms->vy0 = 91;
	init_mouse_cursor8(mcursor,BABYBLUE);
	layer_setbuf(layerms,(unsigned char *)mcursor,x,y,BABYBLUE);
	layer_updown(layman,layerms,255);//always max height
	return layerms;
}
