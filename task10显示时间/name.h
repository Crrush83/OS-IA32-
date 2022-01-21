struct BOOTINFO{
    char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};

//结构体要写在使用的前面吗？即便是函数声明
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
void putfont8(char *vram, int xsize, int x, int y, char color, char *font);     
void init_mouse_cursor8(char *mouse,char bc);//拓印一个鼠标 并且背景颜色由bc决定 将鼠标拓印到一个全局鼠标显存内容变量中去
void putblockVRAM(char *vram, int vxsize, int pxsize,int pysize, int px0, int py0, char *buf, int bxsize);//bxsize 显示宽度 pxsize 缓存宽度
void putfont8_asc(char *vram,int xsize,int x,int y,char c,unsigned char *s);




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



