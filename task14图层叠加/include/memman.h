#include <stdio.h>
#ifndef _M_H
#define _M_H
unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);
unsigned int memtest_sub_asm(unsigned int start, unsigned int end);

/*memory*/
#define MEMMAN_FREES 4090 //free条目的最大记录数
#define MEMMAN_ADDR 0x19000 //因为这个结构体有32KB呀 0x19000 ~ 0x2ffff
struct FREEINFO{
    unsigned int addr,size;
};//大小：8B

struct MEMMAN{
    int frees,maxfrees,lostsize,losts; //maxfrees代表什么？可用内存中的最大一块？
    struct FREEINFO free[MEMMAN_FREES];
};//大小：32B + 4090*8B = 32752 约32KB
void memman_init(struct MEMMAN *man);//内核堆应该是内核自己规划的吧
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man,unsigned int size);
int memman_free(struct MEMMAN *man,unsigned int addr,unsigned int size);
//size stands for bytes
unsigned int memman_alloc_4k(struct MEMMAN *man,unsigned int size);
unsigned int memman_free_4k(struct MEMMAN *man,unsigned int addr,unsigned int size);


#endif