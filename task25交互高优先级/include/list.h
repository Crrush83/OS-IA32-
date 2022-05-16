#include "memman.h"
#ifndef _H_LIST
#define _H_LIST
struct list_node{
    struct list_node *next;
    struct list_node *prev;
};//从head处获取他所嵌入的那个结构？？
//node前一个变量是node之前的字节数
#define container_of(node) (void*)((unsigned int)node -  *((unsigned int*)((unsigned int)node - 0x4)))
#define keyp(container,keyaddr) (void*)((unsigned int)container + (unsigned int)keyaddr)
struct list_node* new_node();
void free_node(struct list_node* node);
struct list_node* list_init();
void list_insert_at(struct list_node * list,struct list_node *node ,int index);
//void* container_of(struct list_node *node);
void list_insert_inorder(struct list_node * list,struct list_node *node ,int keyadd);
void list_insert_inorderbyuint(struct list_node * head,struct list_node * end,struct list_node *node ,unsigned int keyadd);

#endif