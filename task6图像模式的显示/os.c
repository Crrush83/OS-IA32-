#include "name.h"

// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
int main(void) {
  unsigned char *vram = (unsigned char *)0xa0000;
  int xSize = 320, ySize = 200;

  init_palette();

  box_fill8(vram, xSize, COL8_008484, 0, 0, xSize - 1, ySize - 29);
  box_fill8(vram, xSize, COL8_C6C6C6, 0, ySize - 28, xSize - 1, ySize - 28);
  box_fill8(vram, xSize, COL8_FFFFFF, 0, ySize - 27, xSize - 1, ySize - 27);
  box_fill8(vram, xSize, COL8_C6C6C6, 0, ySize - 26, xSize - 1, ySize - 1);

  box_fill8(vram, xSize, COL8_FFFFFF, 3, ySize - 24, 59, ySize - 24);
  box_fill8(vram, xSize, COL8_FFFFFF, 2, ySize - 24, 2, ySize - 4);
  box_fill8(vram, xSize, COL8_848484, 3, ySize - 4, 59, ySize - 4);
  box_fill8(vram, xSize, COL8_848484, 59, ySize - 23, 59, ySize - 5);
  box_fill8(vram, xSize, COL8_000000, 2, ySize - 4, 59, ySize - 3);
  box_fill8(vram, xSize, COL8_000000, 60, ySize - 24, 60, ySize - 3);

  box_fill8(vram, xSize, COL8_848484, xSize - 47, ySize - 24, xSize - 4,
            ySize - 24);
  box_fill8(vram, xSize, COL8_848484, xSize - 47, ySize - 23, xSize - 47,
            ySize - 4);
  box_fill8(vram, xSize, COL8_FFFFFF, xSize - 47, ySize - 3, xSize - 4,
            ySize - 3);
  box_fill8(vram, xSize, COL8_FFFFFF, xSize - 3, ySize - 24, xSize - 3,
            ySize - 3);

    unsigned char *p = (unsigned char *) 0xa0000;
   for (int i = 0; i <= 0xffff; i++) {
     *(p + i) = i & 0x0f;
   }

  for (;;) {
    io_hlt();
  }
}

// int main(void) {
//   unsigned char *p = (unsigned char *) 0xa0000;

//   init_palette();

//   for (int i = 0; i <= 0xffff; i++) {
//     // write_mem8(i, i & 0x0f);
//     *(p + i) = i & 0x0f;
//   }

//   box_fill8(p, 320, COL8_FF0000, 20, 20, 120, 120);
//   box_fill8(p, 320, COL8_00FF00, 70, 50, 170, 150);
//   box_fill8(p, 320, COL8_0000FF, 120, 80, 220, 180);

//   for (;;) {
//     io_hlt();
//   }
// }

