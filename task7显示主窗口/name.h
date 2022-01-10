void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
 void write_mem8(int addr, int data);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void box_fill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0,
               int x1, int y1);

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