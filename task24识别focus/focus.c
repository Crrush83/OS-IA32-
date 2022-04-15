#include "focus.h"
#include "layer.h"
#include "stdio.h"
//初始化为-1 或0 总之 <= 0 代表不点击在任何一个窗口上
//LAYER添加是否是窗口的属性
struct LAYER* focus_layer;
//为了输入中断准备 必须在鼠标点击的中断中
struct task* focus_task;
void focus(struct LAYER* layer){

}
void cancel_focus(struct LAYER* leyer){
    
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
       if(old_focus > 0){
           cancel_focus(old_focus);//也是图层样子的转变
       }
       if(new_focus > 0){
           focus(new_focus);//样子的转变
       }
       focus_layer = new_focus;
       focus_task = focus_layer->task;
       char s[20];
       sprintf(s,"focus change to %d",(int)focus_layer);//点到鼠鼠自己咯。。。top_map不要包含鼠鼠
       debugPrint(s);
   }
    
}