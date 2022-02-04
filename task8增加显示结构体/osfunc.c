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

void box_fill8(char *vram, int xsize, unsigned char c, int x0, int y0,
               int x1, int y1) {
  for (int y = y0; y <= y1; y++) {
    for (int x = x0; x <= x1; x++) {
      vram[y * xsize + x] = c;
    }
  }

  return;
}

void init_screen(char *vram, int x, int y)
{
  int xSize = x,ySize = y;
  int taskBar = y/10+y/100;
  int taskSize = y/10;
  int taskBottom = y/100;

  box_fill8(vram, xSize, BABYBLUE, 0, 0, xSize - 1, ySize - taskBar - 1);
  box_fill8(vram, xSize, MOON, 0, ySize - taskBar - 1, xSize - 1, ySize - 1);
  box_fill8(vram, xSize, WHITE, 0, ySize - taskBar - 1, taskSize - 1, ySize - 1 - taskBottom);
  box_fill8(vram, xSize, MOUSE, 0, ySize - 1 - taskBottom, taskSize - 1, ySize - 1);
}
//显示文字就是拓印
void putfont8(char *vram, int xsize, int x, int y, char color, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = color; }
		if ((d & 0x40) != 0) { p[1] = color; }
		if ((d & 0x20) != 0) { p[2] = color; }
		if ((d & 0x10) != 0) { p[3] = color; }
		if ((d & 0x08) != 0) { p[4] = color; }
		if ((d & 0x04) != 0) { p[5] = color; }
		if ((d & 0x02) != 0) { p[6] = color; }
		if ((d & 0x01) != 0) { p[7] = color; }
	}
	return;
}
