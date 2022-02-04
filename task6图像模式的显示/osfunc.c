#include "name.h"
void init_palette(void) {
  static unsigned char table_rgb[16 * 3] = {
      0x00, 0x00, 0x00, // 黑色
      0xff, 0x00, 0x00, // 亮红色
      0x00, 0xff, 0x00, // 亮绿色
      0xff, 0xff, 0x00, // 亮黄色
      0x00, 0x00, 0xff, // 亮蓝色
      0xff, 0x00, 0xff, // 亮紫色
      0x00, 0xff, 0xff, // 浅亮蓝色
      0xff, 0xff, 0xff, // 白色
      0xc6, 0xc6, 0xc6, // 亮灰色
      0x84, 0x00, 0x00, // 暗红色
      0x00, 0x84, 0x00, // 暗绿色
      0x84, 0x84, 0x00, // 暗黄色
      0x00, 0x00, 0x84, // 暗蓝色
      0x84, 0x00, 0x84, // 暗紫色
      0x00, 0x84, 0x84, // 浅暗蓝色
      0x84, 0x84, 0x84  // 暗灰色
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
