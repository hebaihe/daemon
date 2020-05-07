#include "process_pool.h"
//#define filename "file"
void sigPipeFunc(int signum)
{
signum=0;
}
int trans_file(int newFd,char* filename)
{//忽略SIGPIPE,防止因客户端断开引起子进程崩溃
    signal(SIGPIPE,sigPipeFunc);
    printf("wojin trans_file\n");
    int ret;
    //发送文件名
    train_t dataTrain;
    dataTrain.dataLen=strlen(filename);//规范是要转换大小端
    strcpy(dataTrain.buf,filename);
    send(newFd,&dataTrain,4+dataTrain.dataLen,0);
    //接收对方发来的client端待传送文件大小
    off_t len=0;
    recvCycle(newFd,&len,4);
    //发送待传送文件大小给客户端
    struct stat buf;
    stat(filename,&buf);
    buf.st_size-=len;
    dataTrain.dataLen=sizeof(buf.st_size);
    memcpy(dataTrain.buf,&buf.st_size,dataTrain.dataLen);
    send(newFd,&dataTrain,4+dataTrain.dataLen,0);
    //sendfile发送文件内容 25s
    int fd=open(filename,O_RDWR);
    ERROR_CHECK(fd,-1,"open");
    sendfile(newFd,fd,&len,buf.st_size);//从len开始传,结果验证确实是这样
    ERROR_CHECK(ret,-1,"sendfile");
/*    //mmap发送文件内容
    int fd=open(filename,O_RDWR);
    ERROR_CHECK(fd,-1,"open");
    char *pStart=(char*)mmap(NULL,buf.st_size-len,PROT_READ|PROT_WRITE,MAP_SHARED,fd,len);//没有检验mmap方式的断点续传第二个参数是st_size还是st_size-len
    ERROR_CHECK(pStart,(char*)-1,"mmap");
    send(newFd,pStart,buf.st_size,0);
  */  //发结束标志
   // dataTrain.dataLen=0;
    //send(newFd,&dataTrain,4,0);
    return 0;
}

