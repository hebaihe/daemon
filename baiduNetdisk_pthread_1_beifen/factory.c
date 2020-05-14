#include "factory.h"
//初始化工厂
int factory_init(pFactory_t p, int threadNum, int queCapacity)
{
    bzero(p, sizeof(Factory_t));
    que_init(&p->que, queCapacity);                               //队列初始化--互斥锁：互斥访问任务队列
    pthread_cond_init(&p->cond, NULL);                            //同步信号量初始化--一次唤醒一个子进程拿任务
    p->pthid = (pthread_t *)calloc(threadNum, sizeof(pthread_t)); //为线程id申请threadNum个空间
    p->threadNum = threadNum;
    return 0;
}
//子线程的函数
void *threadFunc(void *p)
{
    pFactory_t pFac = (pFactory_t)p; //拿到main线程传过来的factory（整个世界）
    pQue_t pq = &pFac->que;          //指针指向原任务队列
    pNode_t pTask;                   //任务节点
    while (1)
    { //有|无任务：加锁-拿任务-解锁 | 加锁-等待任务-拿任务-解锁
        pthread_mutex_lock(&pq->queMutex);
        if (!pq->queSize)
        { //wait上半部：放入等待队列-解锁-睡觉 下半部（被signal唤醒）：醒来-加锁 上半部和下半部均为原子操作，不分先后
            pthread_cond_wait(&pFac->cond, &pq->queMutex);
        }
        int ret = que_get(pq, &pTask); //从队列里拿任务，若被其他线程抢到则空转一轮？？为什么会被其他线程抢到:main线程解锁会被其他线程抢到
        pthread_mutex_unlock(&pq->queMutex);
        if (ret)
        {
            trans_file(pTask->newFd); //同一进程内线程共享client端newFd，不需要recvmsg，且没有文件描述符的复制
            // char *a = getcwd(NULL, 0);
            // int len = strlen(a);
            // int type = 6; //pwd为6号命令
            // printf("len=%d\n", len);
            // send(pTask->newFd, &type, 4, 0);
            // send(pTask->newFd, &len, 4, 0);
            // send(pTask->newFd, a, len, 0);
            // printf("%s\n", a);
            close(pTask->newFd);
            free(pTask);
        }
    }
}
//创建子线程，并把主要的数据结构传给每一个子线程
int factory_start(pFactory_t p)
{
    int i;
    if (!p->startFlag)
    {
        for (i = 0; i < p->threadNum; i++)
        { //传出参数p->pthid+i等价于&pthid[i]+子线程函数入口地址+main线程传过来的工厂
            pthread_create(p->pthid + i, NULL, threadFunc, p);
        }
        p->startFlag = 1; //工厂启动啦
    }
    return 0;
}
