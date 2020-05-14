#include "factory.h"

int main(int argc,char *argv[])
{
    if(argc!=5)
    {//一次性接收任务太多会使client等待时间过长，根据负载均衡设置queCapacity
        printf("usage:./server IP PORT threadNum queCapacity\n");
    }
    printf("hello world\n");
    Factory_t facData;
    int threadNum=atoi(argv[3]);
    int queCapacity=atoi(argv[4]);
    //工厂初始化
    factory_init(&facData,threadNum,queCapacity);
    //启动工厂即线程池
    factory_start(&facData);
    //socket-bind-listen
    int socketFd;
     struct sockaddr_in client;
    tcp_init(&socketFd,argv[1],argv[2]);
    int newFd;
    pNode_t pNewRequest;
    pQue_t pQueue=&facData.que;//拿到队列的结构体地址
    while(1)//接受客户端请求并将任务放到任务队列
    {   bzero(&client,sizeof(client));
        socklen_t len=sizeof(client);
        newFd=accept(socketFd,(struct sockaddr *)&client,&len);
        ERROR_CHECK(newFd,-1,"accept");
        printf("client ip=%s,client port=%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
        pNewRequest=(pNode_t)calloc(1,sizeof(Node_t));
        pNewRequest->newFd=newFd;
        pthread_mutex_lock(&pQueue->queMutex);
        que_set(pQueue,pNewRequest);//节点放入队列
        pthread_mutex_unlock(&pQueue->queMutex);
        pthread_cond_signal(&facData.cond);//唤醒一个子线程
    }
    return 0;
}

