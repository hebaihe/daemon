#include "process_pool.h"
int exitFds[2];//退出机制是用的管道
void sigFunc1(int signum){
    write(exitFds[1],&signum,1);
}
int main(int argc,char *argv[])
{   int ret;
    if(argc!=4)
    {
        printf("usage ./process_pool_server IP PORT PROCESS_NUM\n");
        return -1;
    }
    pipe(exitFds);//退出机制的无名管道初始化，fds[0]是读端，1是写端
    signal(SIGINT,sigFunc1);//使用ctrl+c（2号信号）触发子进程退出机制
    int processNum=atoi(argv[3]);//进程数目
    process_data_t *pData=(process_data_t*)calloc(processNum,sizeof(process_data_t));
    make_child(pData,processNum);//创建子进程并初始化数据结构
    int socketFd;
    //为了实现socket，bind,listen
    tcp_init(&socketFd,argv[1],argv[2]);

    //使用epoll监控socketFd与client建立连接，监控子进程管道对端，监控退出机制所用的管道读端
    int epfd=epoll_create(1);
    struct epoll_event event,*evs;
    evs=(struct epoll_event*)calloc(processNum+3,sizeof(struct epoll_event));
    event.events=EPOLLIN;//监控的事件为读事件
    event.data.fd=socketFd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,socketFd,&event);
    int i;
    //监控每一个子进程管道对端
    for(i=0;i<processNum;++i)
    {
        event.data.fd=pData[i].fd;
        epoll_ctl(epfd,EPOLL_CTL_ADD,pData[i].fd,&event);
    }//监控退出所用的管道读端
    event.data.fd=exitFds[0];
    epoll_ctl(epfd,EPOLL_CTL_ADD,exitFds[0],&event);
    int readyFdNum,newFd,j,k;
    struct sockaddr_in client;
    char childBusyFlag=0;
    char buf[100]={0};
    while(1)
    {
        readyFdNum=epoll_wait(epfd,evs,processNum+3,-1);//-1是无限等待
        for(i=0;i<readyFdNum;++i)
        {
            //有客户端连接请求,只负责与客户端建立连接，之后客户端的所有请求交给子进程处理
            if(evs[i].data.fd==socketFd)
            {
                bzero(&client,sizeof(client));
                socklen_t len=sizeof(client);
                newFd=accept(socketFd,(struct sockaddr *)&client,&len);
                ERROR_CHECK(newFd,-1,"accept");
                printf("client ip=%s,client port=%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                for(j=0;j<processNum;++j)
                {
                    if(0==pData[j].busy)
                    {
                        sendFd(pData[j].fd,newFd);//发newFd给子进程，newFd引用计数+1
                        //发送对应客户端任务给子进程
                        printf("main process give %d task to %d\n",newFd,pData[j].pid);
                        k=pData[j].fd;
                        pData[j].busy=1;//设置为忙碌
                        break;
                    }
                }
                close(newFd);//减少引用计数
            }
            //子进程不忙碌通知父进程
            for(j=0;j<processNum;++j)
            {
                if(evs[i].data.fd==pData[j].fd)//判断是哪一个子进程
                {
                    read(pData[j].fd,&childBusyFlag,1);//把数据读出来
                    pData[j].busy=0;//状态改为非忙碌
                    printf("%d is not busy\n",pData[j].pid);//打印协助理解
                    //event.data.fd=newFd;
                    //epoll_ctl(epfd,EPOLL_CTL_DEL,newFd,&event);//删除对new_fd的监听
                    //close(newFd);
                    break;
                }
            }
            if(evs[i].data.fd==exitFds[0]){//子进程退出机制:收到了要求退出的10号信号
            event.events=EPOLLIN;
            event.data.fd=socketFd;
            epoll_ctl(epfd,EPOLL_CTL_DEL,socketFd,&event);
            close(socketFd);
                for(j=0;j<processNum;++j){
                    sendFd(pData[j].fd,-1);//发送fd=-1代表结束
                }
                for(j=0;j<processNum;++j){
                    wait(NULL);
                    printf("child exit success\n");
                }
                exit(0);
            }    
        }
    }
    return 0;
}