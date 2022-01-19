#include "name.h"

// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
int main(void) {
  struct BOOTINFO *binfo = (struct BOOTINFO*)0x0ff0;//内存分布图0x0ff0开始应该顺次填入BOOTINFO的内容
 // extern char hankaku[4096];
  char mcursor[256];
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	//show a string
    putfont8_asc(binfo->vram, binfo->scrnx,  8, 8, ROSE, "show string API");
	//show a cursor!
	init_mouse_cursor8(mcursor,BABYBLUE);
	putblockVRAM(binfo->vram,binfo->scrnx,16,16,mx,my,mcursor,16);
 	 for (;;) {
    io_hlt();
  }
}



