#ifndef _IO_H
#define _IO_H


//结构体要写在使用的前面吗？即便是函数声明
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_in8(int port);
int io_load_eflags(void);
void io_store_eflags(int eflags);
int load_cr0(void);
void store_cr0(int cr0);
void write_mem8(int addr, int data);
void write_mem32(int addr, int data);

#endif