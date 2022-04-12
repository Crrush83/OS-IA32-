#include "list.h"
struct list_node* new_node(){
    return (struct list_node*)memman_alloc(8);//
}
void free_node(struct list_node* node){ 
    memman_free((unsigned int)node,8);//
}
struct list_node* list_init(){
    struct list_node *list = new_node();//0x149108
    list->next =new_node();//head 0x149110//
    list->prev = new_node(); //118
    list->prev->next = list->next;
    list->prev->prev = list->next;
    list->next->prev = list->prev;
    list->next->next = list->prev;
    return list;
}
void list_insert_at(struct list_node * list,struct list_node *node ,int index){
    int count = 1;
    struct list_node *pre = list;//dump head;0x149110
    for(;count < index;count++){
        pre = pre->next;
    }
    struct list_node * next = pre->next;
    pre->next = node;
    node->prev = pre;
    node->next = next;
    next->prev = node;
}
void list_insert_inorderbyuint(struct list_node * head,struct list_node * end,struct list_node *node ,unsigned int keyadd){
    unsigned int insertkey = *(unsigned int*)(keyp(container_of(node),keyadd));
    struct list_node *ne = head->next;
    for(;ne!=end;ne = ne->next){
        unsigned int nekey = *(unsigned int*)(keyp(container_of(ne),keyadd));
        if(nekey > insertkey) break;
    }
    //插在ne之前哈
    struct list_node *pre = ne->prev;
    pre->next = node;
    node->prev = pre;
    node->next = ne;
    ne->prev = node;
}
// void* container_of(struct list_node *node){
//     int presize = node - *(int *)(node - 4);
//     return (void*)(node - presize);
// }
// void* container_of(struct list_node * node){
//     return (void*)((unsigned int)node -  *((unsigned int*)((unsigned int)node - 0x4)));
// }
//这样才是对的？？