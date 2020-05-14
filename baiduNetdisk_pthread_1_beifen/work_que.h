#ifndef __WORK_QUE_H__
#define __WORK_QUE_H__
#include "head.h"

typedef struct tag_node{//任务队列节点，用于存放客户端请求
    int newFd;
    struct tag_node *pNext;
}Node_t,*pNode_t;

typedef struct{//任务队列，链表实现，用于存放客户端请求
    pNode_t queHead,queTail;
    int queCapacity;//队列能力
    int queSize;//队列当前的元素个数
    pthread_mutex_t queMutex;//创建一把队列锁用于互斥接受任务
}Que_t,*pQue_t;
int que_init(pQue_t,int);//初始化任务队列
int que_set(pQue_t,pNode_t);//入队，向任务队列放入任务节点
int que_get(pQue_t,pNode_t*);//出队，从任务队列拿出任务节点
#endif
