//unset cache
//if p6 CR0 CD 1 NW 0
//if 4
/*The Pentium 4 processor does not support this mode; setting the CD and NW bits to 1 selects the no-fill
cache mode.
即CR0 30位：1 29位1
CR0 |= 0x60000000
*/
//else CR0 |= 0x4000 0000
//No-fill Cache Mode. Memory coherency is maintained.
/*
The effect of setting the CD flag is somewhat different for the Pentium 4 and
P6 family processors than for the Pentium processor (see Table 9-5). To
insure memory coherency after the CD flag is set, the caches should be
explicitly flushed (see Section 9.5.3., “Preventing Caching”).
*/
#include "io.h"
#include "memman.h"
#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000 //0 1 0 0 
unsigned int memtest(unsigned int start, unsigned int end)
{
	char Pentium4 = 0;
	unsigned int eflg, cr0;
	unsigned int i;

	//
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { 
		Pentium4 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
	io_store_eflags(eflg);

	if (Pentium4!= 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; //set cr0 30:1 29:1 禁止缓存
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);//返回最后的end

	if (Pentium4 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; 
		store_cr0(cr0);
	}

	return i ;//有点想死捏 会编写的怎么也错了
}

//为什么汇编和C都不行？难道没法用*访存？
//可是我没在汇编里看到异或的代码啊？奇怪
unsigned int memtest_sub(unsigned int start, unsigned int end) {
  unsigned int i, *p, old;
  int pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
  	unsigned int size = 0;
  for (i = start; i <= end; i += 0x1000) {
    p = (unsigned int *)(i + 0xffc);

    old = *p;         // 先记住修改前的值
     // 试写
    *p = pat0;        // 试写
    *p ^= 0xffffffff; // 反转
	
	//return *p; // 为什么*p打印出是0xffff ffff old是0 没有写入成功嘛？哈哈
    if (*p != pat1) {
      // 检查反转结果
    not_memory:
      *p = old;
      break;
    }

    *p ^= 0xffffffff; // 再次反转
    if (*p != pat0) {
      // 检查值是否恢复
      goto not_memory;
    }
    *p = old;
	size += 0x1000;
  }
  return size;
}
//初始化：还没有规划哪一块可用 所以内存可用总量为0 
void memman_init(struct MEMMAN *memman){
    memman->frees = 0;//free item num
    memman->maxfrees = 0;
    memman->lostsize = 0;
    memman->losts = 0;
    return;
}

unsigned int memman_total(){
    extern struct MEMMAN *memman;
    unsigned int i,sum = 0;
    for(i = 0;i < memman->frees;i++){
        sum+=memman->free[i].size;
    }
    return sum;
}

//use a chunk of memory
//should recored 
//renew where are usable
unsigned int memman_alloc(unsigned int size){
    extern struct MEMMAN *memman;
    int i;
    for(i = 0;i < memman->frees;i++){
        if(memman->free[i].size >= size){
            //find a chunk 最先发现模式
            break;
        }
    }   
	 if(i == memman->frees){
        return 0;
    }
    int addr = memman->free[i].addr;
    

    
    memman->free[i].size -= size;
    memman->free[i].addr += size;
    if(memman->free[i].size == 0){
        //delete this record item
            memman->frees--;
        for(;i < memman->frees;i++){
        memman->free[i] = memman->free[i+1];}
    }
    return addr;
}

int memman_free(unsigned int addr,unsigned int size){
    extern struct MEMMAN *memman;
    int i;
	//frees项都是不重叠的 所以先找到一个比欲插入项大的
    for(i = 0;i < memman->frees;i++){
        if(memman->free[i].addr >= addr){
            break;
        }
    }
    //i == 0
        //free chunk can merge to item i :addr+size = man->free[0].addr
        //free chunk cant merge to item i :addr+size != man->free[0].addr
            //new an item
            //have space to emplace : man->frees < MEMMAN_FREES
            //[lost]no space :lost
    //0 < i < man->frees 
        //free chunk can merge to item i-1
            //merge item i-1 to i :delete item i
            //cant merge to i:donothing
        //free chunk cant merge to i-1
            //can merge to i :doit
            //cant merge to i : new a item
			    //if no space
                //[lost]man->free == MEMMAN_FREES
                //man->free < MEMMAN_FREES
           
    //i == man->frees    
        //free chunk can merge to item i-1
        //[lost]free chunk cant merge to item i-1 :new an item but!lost!
        int index;
    if(i == 0 && addr+size == memman->free[0].addr){
        memman->free[0].size += size;
        return 0;
    }
    else if(i == 0 && addr+size != memman->free[0].addr){
        if(memman->frees < MEMMAN_FREES){
            //insert new item
            memman->frees++;
            for(index = memman->frees - 1;index > i;index--){
                memman->free[index] = memman->free[index-1];
            }
            memman->free[i].size = size;
            memman->free[i].addr = addr;
            return 0;
         }
        else{
            goto lost;
        }
    }
    else if(0 < i && i < memman->frees && memman->free[i-1].addr + memman->free[i-1].size == addr && memman->free[i].addr == addr+size){
        //delete item  i
        memman->free[i-1].size += size;
        memman->free[i-1].size += memman->free[i].size;
        memman->frees--;
        for(index = i;index < memman->frees;index++){
            memman->free[index] = memman->free[index+1];
        }
        return 0;
    }
    else if(0 < i && i < memman->frees && memman->free[i-1].addr + memman->free[i-1].size == addr && memman->free[i].addr != addr+size){
        //merge to i-1
        memman->free[i-1].size += size;
        return 0;
    }
    else if(0 < i && i < memman->frees && memman->free[i-1].addr + memman->free[i-1].size != addr && memman->free[i].addr == addr+size){
        //merge to i
        memman->free[i].addr -= size;
        memman->free[i].size += size;
        return 0;
    }
    else if(0 < i && i < memman->frees && memman->free[i-1].addr + memman->free[i-1].size != addr && memman->free[i].addr != addr+size){
        if(memman->frees < MEMMAN_FREES){
            //insert new item
            memman->frees++;
            for(index = memman->frees - 1;index > i;index--){
                memman->free[index] = memman->free[index-1];
            }
            memman->free[index].size = size;
            memman->free[index].addr = addr;
            return 0;
        }
        else{
            goto lost;
        }
    }
    else if(i == memman->frees && memman->free[i-1].addr+size == addr){
            //merge to i-1
            memman->free[i-1].size += size;
            return 0;
    }
    else if(i == memman->frees && memman->free[i-1].addr+size != addr){
        if(memman->frees < MEMMAN_FREES){
            //insert new item
            memman->frees++;
            memman->free[i].addr = addr;
            memman->free[i].size = size;
            return 0;
        }
        else{
            goto lost;
        }
    }
    lost:
        memman->lostsize+=size;
        memman->losts++;
        return -1;
        
    return -1;
}

//size stands for bytes
unsigned int memman_alloc_4k(unsigned int size){
    unsigned int a;
    //divide 4k eqaul to & 
    size = (size + 0xfff) & 0xfffff000;//凡不是4k的整倍数的都要向上取整
    a = memman_alloc(size);
    return a;
}
unsigned int memman_free_4k(unsigned int addr,unsigned int size){
    size = (size + 0xfff) & 0xfffff000;
    return memman_free(addr,size);
}