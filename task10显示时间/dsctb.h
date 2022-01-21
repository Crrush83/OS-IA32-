struct INTERUPT_GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
#define IDT 0x17e00
//struct 定义在使用前 我真的服了
//中断
void init_idt(void);
void set_intgatedesc(struct INTERUPT_GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_idtr(int limit, int addr);//现写内存 然后再设定中断表寄存器 见doc文档
