#include "focus.h"
#include "layer.h"
#include "stdio.h"
//初始化为-1 或0 总之 <= 0 代表不点击在任何一个窗口上
//LAYER添加是否是窗口的属性
struct LAYER* focus_layer;
//为了输入中断准备 必须在鼠标点击的中断中
struct TASK* focus_task;
//图层（显示）意义上的绘制focus
void focus(struct LAYER* layer){
    //height == 0是背景
    if(layer->height > 0){
    draw_window_title(layer->buf,layer->bxsize,layer->title,1);
    //还要刷新呀！！
    layers_refresh_v3(layer->height,layer->vx0,layer->vy0,layer->vx0 + layer->bxsize ,layer->vy0 + 20);
    //只刷新标题区域
    }
}
void cancel_focus(struct LAYER* layer){
    if(layer->height > 0){
    draw_window_title(layer->buf,layer->bxsize,layer->title,0);
    layers_refresh_v3(layer->height,layer->vx0,layer->vy0,layer->vx0 + layer->bxsize ,layer->vy0 + 20);
    }
}
void check_focus(int mousex,int mousey){
    //extern mouse_x mouse_y
    //old_focus
    //get focus
    //现在focus的图层
    /*
    */
   if(!(mousex >= 0 && mousex < 320 && mousey >=0 && mousey <200)) return;
   extern struct LAYER** top_map;
   struct LAYER* old_focus = focus_layer;
   extern struct BOOTINFO *binfo;
   struct LAYER* new_focus = top_map[mousey * binfo -> scrnx +  mousex];//范围内
   if(old_focus == new_focus){
    //   debugPrint("focus no change");
   }else{
           cancel_focus(old_focus);//也是图层样子的转变
           focus(new_focus);//样子的转变

       focus_layer = new_focus;
       //如果新的focus_layer是无效值？（会吗）
       //如果新的focus_layer不绑定task（绑定的task为NULL） 输入框的输入会废弃（是的）
       //修改输入逻辑：fucos_task == null 或者 focus_task->fifo == null放弃输
       
       focus_task = focus_layer->task;
       //char s[20];
       //sprintf(s,"focus change to %d",(int)focus_layer);//点到鼠鼠自己咯。。。top_map不要包含鼠鼠
       //debugPrint(s);
   }
    
}