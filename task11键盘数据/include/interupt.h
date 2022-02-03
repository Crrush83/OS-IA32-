struct INTERUPT_GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

//struct 定义在使用前 我真的服了
//中断
void init_idt(void);
void set_intgatedesc(struct INTERUPT_GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_idtr(int limit, int addr);//现写内存 然后再设定中断表寄存器 见doc文档
void init8259A(void);//可编程中断控制器
void int0x70(void);
void int0x74(void);
void int0x21(void);
void exception(void);
void exceptionprint(void);
void dealerrorcode(void);

void init_gdt(void);
void load_gdtr(void);

