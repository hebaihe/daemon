#include "work_que.h"
//任务队列初始化
int que_init(pQue_t pq,int Capacity)
{
    bzero(pq,sizeof(Que_t));
    pq->queCapacity=Capacity;//数组，指针，结构体->能实现值双向传递
    pthread_mutex_init(&pq->queMutex,NULL);
    return 0;
}
//节点入队，链式队列采用尾插法实现队尾进对头出       
int que_set(pQue_t pq,pNode_t pNew)
{   //入队判空
    if(!pq->queSize)
    {
        pq->queHead=pNew;
        pq->queTail=pNew;
    }else{
        pq->queTail->pNext=pNew;
        pq->queTail=pNew;
    }
    pq->queSize++;
    return 0;
}
//从任务队列拿任务，即节点出队时判空判1节点
int que_get(pQue_t pq,pNode_t *pTask)
 {
    if(pq->queSize)
    {
        *pTask=pq->queHead;
        pq->queHead=pq->queHead->pNext;
        pq->queSize--;
        if(NULL==pq->queHead)//判1节点
        {
            pq->queTail=NULL;
        }
        return 1;
    }
    return 0;
}
