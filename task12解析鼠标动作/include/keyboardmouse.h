void wait_KBC_sendready(void);
void init_keyboard(void);
void enable_mouse(void);
void showmouse(void);
void savekbdata(void);
void savemousedata(void);
struct MOUSE_DEC {
	unsigned char buf[3], phase;
};