#include "layer.h"
#include "memman.h"
#include "graphic.h"
#include <stdio.h>
struct LAYERMAN *layman;
struct LAYER* backgroundlayer;
struct LAYER** top_map;//需要在layer_init里分配内存
/*
分配图层管理器内存 初始化所有图层为未使用 填充topmap为-1
*/
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
	top_map = (struct LAYER**)memman_alloc(320*200*4);//
	int j;
	for(i = 0;i < 320;i++){
		for(j = 0;j < 200;j++){
			top_map[j * 320 + i] = (struct LAYER*)0;
		}
	}
	/*focus init*/
	extern struct LAYER* focus_layer;
	extern struct TASK* focus_task;
	focus_layer = (struct LAYER*)NULL;
	focus_task = (struct TASK*)NULL;
	err:
		return layerman;
}

struct LAYER *layer_register()
{
	struct LAYER *layer;
	int i;
	for (i = 0; i < MAX_LAYER_NUM; i++) {
		if (layman->layerset[i].flags == 0) {
			layer = &(layman->layerset[i]);
			layer->flags = LAYER_USE; 
			layer->height = -1;
			layer->task = NULL;//还没有绑定任务！ 
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
void layer_updown(struct LAYER *layer, int height)
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
	layers_refresh_v3(0,layer->vx0,layer->vy0,layer->vx0 + layer->bxsize,layer->vy0+layer->bysize);
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
		layers_refresh_v3(height,layer->vx0,layer->vy0,layer->vx0 + layer->bxsize,layer->vy0+layer->bysize);

	}
	return;
}

void layers_refresh()
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
				if(layer->height < layman->top) top_map[vy * layman->binfo->scrnx + vx] = layer;
			}
		}
	}
	return;
}
//刷新区域、图层溢出
//vx0:较小（靠左）的在vram中的水平向起点
//bx:在buf中的水平向坐标
void layers_refresh_v2( int vx0, int vy0, int vx1, int vy1)
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
				if(layer->height < layman->top) top_map[vy * layman->binfo->scrnx + vx] = layer;
			}
		}
	}
	return;
}


void layers_refresh_v3(int h0, int vx0, int vy0, int vx1, int vy1)
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
			if(vy<0 ||vy >= 200){continue;}
			for (bx = bx0; bx <= bx1; bx++) {
				vx = layer->vx0 + bx;
				if(vx <0 ||vx >= 320){continue;}
				c = buf[by * layer->bxsize + bx];//不要超过 16 * 16 -1 小心咯
				if (c != layer->col_inv) {
					vram[vy * layman->binfo->scrnx + vx] = c;
				}
				//制作topmap
				if(layer->height < layman->top) top_map[vy * layman->binfo->scrnx + vx] = layer;
				
			}
		}
	}
	return;
}

void layer_slide( struct LAYER *layer, int vx0, int vy0)
{
	if (layer->height >= 0) { 
		//ab
		int oldx = layer->vx0,oldy = layer->vy0;
		layer->vx0 = vx0;
	    layer->vy0 = vy0;
        layers_refresh_v3(0,oldx,oldy,
		oldx+layer->bxsize - 1,oldy+layer->bysize -1); 
		layers_refresh_v3(layer->height,vx0,vy0,
		 vx0+ layer->bxsize-1,vy0+layer->bysize -1);
	}else{
	layer->vx0 = vx0;
	layer->vy0 = vy0;}
	return;

}
//高度-1 目的在ordered里空出位置
//同时标记为未使用 在layserset里空出位置
void layer_free( struct LAYER *layer)
{
	if (layer->height >= 0) {
		layer_updown( layer, -1); 
	}
	layer->flags = 0; 
	return;
}

/*背景图层*/
struct LAYER* layer_screen(int x, int y)
{
	//mem space 
	unsigned char * bgbuf = (unsigned char * )memman_alloc_4k(x * y);
	init_screen(bgbuf,x,y);
	struct LAYER * layerbg;
	layerbg = layer_register(layman);
	layerbg->task = NULL;//任何一个进程都处理鼠标吗？ 没必要
	layer_setbuf(layerbg,(unsigned char *)bgbuf,x,y,16);
	layer_updown(layerbg,0);
	//默认 0，0 位置
	return layerbg;
}
/*鼠标图层*/
struct LAYER* layer_mouse(int x,int y){
	unsigned char * mcursor = (unsigned char *)memman_alloc(256);//是全局变量还是堆上变量 应该不影响
  //  extern unsigned char *mcursor;
	struct LAYER * layerms;
	layerms = layer_register(layman);
	layerms->vx0 = 0;
	layerms->vy0 = 0;
	init_mouse_cursor8(mcursor,BABYBLUE);
	layer_setbuf(layerms,(unsigned char *)mcursor,x,y,BABYBLUE);
	layer_updown(layerms,layman->top+1);//always max height
	return layerms;
}
#define CLOSE 0x0
#define CLOSEFOCUS 0x1
#define MINI 0x2
#define MINIFOCUS 0x3
//画窗口样式 同时将title写入layer结构
void draw_window(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	//窗体底色
	box_fill8(buf, xsize,MOON, 0,0, xsize - 1, ysize - 1);
	//title_bind -> draw window -> draw title
	draw_window_title(buf,xsize,title,act);
	//绘制按钮
	char c;
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = ROSE;
			} else if (c == '$') {
				c = PINK;
			} else if (c == 'Q') {
				c = PINK;
			} else {
				c = PINK;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}
void draw_window_title(unsigned char *buf,int xsize,char *title, char act){
	char tc, tbc;
	if (act != 0) {
		tc = MOON;
		tbc = MOUSE;
	} else {
		tc = MOUSE;
		tbc = MOON;
	}
	//窗口标题背景颜色
	box_fill8(buf, xsize, tbc,4,4,xsize-24,18);
	//窗口标题颜色
	putfont8_asc(buf, xsize, 24,5, tc, title);
}
void layer_settitle(struct LAYER *layer,unsigned char *title){
	//挂标题独立一个函数 比较漂亮。
	//嵌入标题属性
	int wp = 0;
	for(;wp < 29;wp++){
		if(title[wp]!='\0'){
		layer->title[wp] = title[wp];
		}else{
		layer->title[wp] = title[wp];
		break;
		}
	}
	layer->title[29] = '\0';
}
//绘制一个窗口
//规定大小：114*52
//layer
//因为layer必须关联task 所以这个过程暂时不用了
//拆分为申请图层+申请buf区域+绑定Buf+在Buf上绘制窗口+给定窗口图层绘制标题+和task互相绑定
struct LAYER* layer_window(int posix,int posiy,int sizex,int sizey){
	extern struct LAYERMAN *layman;
	unsigned char * window = (unsigned char *)memman_alloc_4k(sizex*sizey);
	struct LAYER *layer;
	layer = layer_register(layman);
	layer->vx0 = posix;
	layer->vy0 = posiy;
	draw_window(window,sizex,sizey,"window",1);
	layer_setbuf(layer,window,sizex,sizey,-1);
	make_textbox8(layer,4,20,sizex - 5,sizey - 4,WHITE);
	layer_updown(layer,layman->top+1);
	return layer;
}
void make_textbox8(struct LAYER* layer, int x0, int y0, int x1, int y1, int c)
{
	box_fill8(layer->buf, layer->bxsize, MOUSE, x0, y0,   x1, y0);//上
	box_fill8(layer->buf, layer->bxsize, MOUSE, x0, y0,   x0, y1);//左
	box_fill8(layer->buf, layer->bxsize, BLACK, x0+1,y0+1,x1-1,y0+1);
	box_fill8(layer->buf, layer->bxsize, BLACK, x0+1,y0+1,x0+1,y1-1);
	box_fill8(layer->buf, layer->bxsize, WHITE, x0+1, y1, x1-1,y1);//下
	box_fill8(layer->buf, layer->bxsize, WHITE, x1, y0,   x1, y1);//右
	box_fill8(layer->buf, layer->bxsize, MOUSE, x0+1,y1-1,x1-1,y1-1);
	box_fill8(layer->buf, layer->bxsize, MOUSE, x1-1,y0+1,x1-1,y1-1);
	box_fill8(layer->buf, layer->bxsize, c,x0+2,y0+2,x1-2,y1-2);
	return;
}

//x y是相对layer的
void putstr_on_layer(struct LAYER *layer, int x, int y, int c, int b, char *s,int strlen)
{
	extern struct LAYERMAN *layman;
	//刷背景
	box_fill8(layer->buf, layer->bxsize, b, x, y, x + strlen * 8 - 1, y + 15);
	//刷字
	putfont8_asc(layer->buf, layer->bxsize, x, y, c, (unsigned char*)s);
	//为啥不刷？ 
	layers_refresh_v3(layer->height,x+layer->vx0, y+layer->vy0, layer->vx0+x + strlen * 8 - 1, 
	layer->vy0+y + 16 - 1);
	//layers_refresh_v3(layman,0,20,20,200,100);
	return;
}
void putcursor_on_layer(struct LAYER *layer, int x, int y, int c)
{
	extern struct LAYERMAN *layman;
	//刷背景)
	box_fill8(layer->buf, layer->bxsize, c, x, y, x + 7, y + 15);
	//为啥不刷？ 
	layers_refresh_v3(layer->height,x+layer->vx0, y+layer->vy0, layer->vx0+x + 7, layer->vy0+y + 15);
	return;
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
	layer_updown(layer,layman->top+1);//always max height
	return layer;
}