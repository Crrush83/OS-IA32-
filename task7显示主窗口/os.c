#include "name.h"

// 入口函数HariMain重命名为标准的main
// 返回类型修改为int，避免编译器警告
int main(void) {
  unsigned char *vram = (unsigned char *)0xa0000;
  int xSize = 320, ySize = 200;
  int taskSize = 20,taskBottom = 2,taskBar = 22;

  
  init_palette();

  box_fill8(vram, xSize, BABYBLUE, 0, 0, xSize - 1, ySize - taskBar - 1);
  box_fill8(vram, xSize, MOON, 0, ySize - taskBar - 1, xSize - 1, ySize - 1);
  box_fill8(vram, xSize, WHITE, 0, ySize - taskBar - 1, taskSize - 1, ySize - 1 - taskBottom);
  box_fill8(vram, xSize, MOUSE, 0, ySize - 1 - taskBottom, taskSize - 1, ySize - 1);



 // unsigned char *p = (unsigned char *) 0xa0000;
 //  for (int i = 0; i <= 0xffff; i++) {
   //  *(p + i) = (i>>12); //显示16种颜色
  // }

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

