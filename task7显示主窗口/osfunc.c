#include "name.h"
void init_palette(void) {
  static unsigned char table_rgb[16 * 3] = {
      0x00, 0x00, 0x00, // 黑色
      0xff,0xcd,0xd2,//浅粉
      0xE9,0x1E,0x63,// 玫红
      0xB3,0x9D,0xDB, // 藕荷色
      0xE1,0xF5,0xFE,//baby蓝
      0x4D,0xD0,0xE1, // 松石绿
      0x00, 0xff, 0xff, // 浅亮蓝色
      0xff, 0xff, 0xff, // 白色
      0xF5,0xF5,0xF5, // 月岩色
      0x4C,0xAF,0x50, // 草绿
      0xCD,0xDC,0x39, // 芥末色
      0xFF,0xCA,0x28, //芒果黄
      0xBC,0xAA,0xA4,//烟咖色
      0x90,0xA4,0xAE,//青灰色
      0xFF,0x98,0x00,//桔色
      0x9E,0x9E,0x9E//鼠灰色
  };

  set_palette(0, 15, table_rgb);

  return;
}

void set_palette(int start, int end, unsigned char *rgb) {
  int eflags = io_load_eflags(); // 记录标志

  io_cli(); // 禁止中断

  io_out8(0x03c8, start);
  for (int i = start; i <= end; i++) {
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }

  io_store_eflags(eflags);

  return;
}

void box_fill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
               int x1, int y1) {
  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      vram[y * xsize + x] = c;
    }
  }

  return;
}
