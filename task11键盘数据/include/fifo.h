struct FIFO8{
    unsigned char *buf;
    int nextr,nextw,size,empty,avalible;
};
void fifo8_init(struct FIFO8 *fifo,int size,unsigned char *buf);
int fifo8_put(struct FIFO8 *fifo,unsigned char c);
int fifo8_get(struct FIFO8 *fifo);
int fifo8_status(struct FIFO8 *fifo);