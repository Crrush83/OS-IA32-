struct FIFO32{
    int *buf;
    int nextr,nextw,size,empty,avalible;
};
void fifo32_init(struct FIFO32 *fifo,int size,int *buf);
int fifo32_put(struct FIFO32 *fifo,int c);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);