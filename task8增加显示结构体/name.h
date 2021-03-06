void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
 void write_mem8(int addr, int data);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void box_fill8(char *vram, int xsize, unsigned char c, int x0, int y0,
               int x1, int y1);
void init_screen(char *vram, int x, int y);          
void putfont8(char *vram, int xsize, int x, int y, char color, char *font);   //拓印一个文字 但是并不修改背景部分的显存
#define BLACK 0 // 黑色
#define PINK 1
#define ROSE 2
#define LAVENDER 3
#define BABYBLUE 4
#define MINT 5
#define MINTBLUE 6
#define WHITE 7
#define MOON 8
#define GRASS 9
#define MUSTARD 10
#define MANGO 11
#define COFFEE 12
#define CHINESEINK 13
#define ORANGE 14
#define MOUSE 15

//结构体声明 然后只定义一次?

struct BOOTINFO{
    char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};