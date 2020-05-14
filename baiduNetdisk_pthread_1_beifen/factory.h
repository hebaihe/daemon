#ifndef __FACTORY_H__  //避免同一个头文件被 include 多次
#define __FACTORY_H__  //此处定义的宏代表头文件的宏名
#include "head.h"
#include "work_que.h"
typedef struct{
    Que_t que;
    pthread_cond_t cond;//条件变量，控制同步
    pthread_t *pthid;//存储子线程ID的起始地址
    int threadNum;//线程数目
    int startFlag;//工厂是否启动
}Factory_t,*pFactory_t;
typedef struct{
    int dataLen;//存储的是 buf上到底运了多少数据
    char buf[1000];//存储要发送的数据
}train_t;
int factory_init(pFactory_t,int,int);//工厂初始化
int factory_start(pFactory_t);//工厂开业，创建子线程并把整个工厂的的成员传给它
int tcp_init(int*,char*,char*);//socket-bind-listen
int trans_file(int);//同一进程内线程共享client端newFd，不需要recvmsg
#endif
