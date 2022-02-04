#include "name.h"

// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
int main(void) {
  struct BOOTINFO *binfo = (struct BOOTINFO*)0x0ff0;//内存分布图0x0ff0开始应该顺次填入BOOTINFO的内容
  extern char hankaku[4096];

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    putfont8(binfo->vram, binfo->scrnx,  8, 8, ROSE, hankaku + 'z' * 16);
	putfont8(binfo->vram, binfo->scrnx, 16, 8, ROSE, hankaku + 'x' * 16);
	putfont8(binfo->vram, binfo->scrnx, 24, 8, ROSE, hankaku + 't' * 16);
	putfont8(binfo->vram, binfo->scrnx, 40, 8, ROSE, hankaku + ' ' * 16);
	putfont8(binfo->vram, binfo->scrnx, 48, 8, ROSE, hankaku + '2' * 16);
	putfont8(binfo->vram, binfo->scrnx, 56, 8, ROSE, hankaku + '0' * 16);
    putfont8(binfo->vram, binfo->scrnx, 64, 8, ROSE, hankaku + '2' * 16);
	putfont8(binfo->vram, binfo->scrnx, 72, 8, ROSE, hankaku + '2' * 16);

  for (;;) {
    io_hlt();
  }
}



