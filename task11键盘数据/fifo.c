#include "fifo.h"
#include "io.h"
/*
初始化一个缓冲区 接收缓冲区的指针 extern ... fifo 再传入&fifo初始化这样
指定的大小应该和用于接收数据的buf大小一样
*/
void fifo8_init(struct FIFO8 *fifo,int size,unsigned char *buf){
    fifo->buf = buf;
    fifo->size = size;
    fifo->avalible = size;
    fifo->empty = 1;
    fifo->nextr = 0;//满状态（avalible == 0）可以r不可以w
    fifo->nextw = 0;//空状态(empty = 1) 可以w 不可以r
}
/*
返回-1则失败 否则返回写入的asci码
*/
int fifo8_put(struct FIFO8 *fifo,unsigned char c){
    if(fifo->avalible){
        fifo->avalible--; /*01627686205e[CPU0  ] check_cs(0x148f): not a valid code segment !*/
   //这些打印不出来
   // extern struct BOOTINFO *binfo;
   // char *tmp = "in put";
   // putfont8_asc(binfo->vram, binfo->scrnx, 8,16, ROSE,(unsigned char*)tmp);
   
        fifo->buf[fifo->nextw] = c;
        fifo->nextw++;
        if(fifo->nextw == fifo->size){
            fifo->nextw = 0;
        }
        if(fifo->empty){
        fifo->empty = 0;
    }
    return (int)c;
    }else{
        return -1;
    }
    return -1;
}
int fifo8_get(struct FIFO8 *fifo){
    if(fifo->empty){
        return -1;
    }else{
        int ret = (int)fifo->buf[fifo->nextr];
        fifo->nextr++;
        if(fifo->nextr == fifo->size){
            fifo->nextr = 0;
        }
        fifo->avalible++;
        if(fifo->avalible == fifo->size){
            fifo->empty = 1;
        }
        return ret;
    }
    return -1;
}
int fifo8_status(struct FIFO8 *fifo){
    return (fifo->size - fifo->avalible);
}