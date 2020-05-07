//这个宏定义是因为splice不是Posix标准，属于新扩展的功能么
#define _GNU_SOURCE
#include "process_pool.h"
int recvCycle(int newFd,void *pStart,int len)
{
    char *p=(char*)pStart;
    int total=0,ret;
    while(total<len)
    {
        ret=recv(newFd,p+total,len-total,0);
        if(0==ret)
        {
            printf("byebye\n");
            return -1;
        }
        total=total+ret;
    }
    return 0;
}

int recv_file(int socketFd){
 int dataLen;
    char buf[1000]={0};
    //先接文件名
    recvCycle(socketFd,&dataLen,4);//拿火车头
    recvCycle(socketFd,buf,dataLen);//拿车厢
    int fd=open(buf,O_RDWR|O_CREAT|O_EXCL,0666);//文件若不存在则建立该文件，否则将导致打开文件错误

    /*int i=1;
    char tmp[1000];
    strcpy(tmp,buf);
    while((fd=open(tmp,O_RDWR|O_CREAT|O_EXCL,0666))==-1)
    {
        bzero(tmp,sizeof(tmp));
        sprintf(tmp,"%s%d",buf,i);
        i++;
    }*/
    //接收文件大小
    off_t fileSize,fileSlice,lastLoadSize=0,downLoadSize=0,sendToFile=0,total=0,recvNum;
    //int num=0,repnum=1;
    //double now=0.0;
    recvCycle(socketFd,&dataLen,4);
    recvCycle(socketFd,&fileSize,dataLen);
    printf("收到的文件大小为%ld\n",fileSize);
    fileSlice=fileSize/10000;
    //接文件内容
    struct timeval start,end;
    gettimeofday(&start,NULL);
  /*  //先做出对应大小的文件，不然在用户态对地址空间的改变不会映射到文件，发送方不需要，ftruncate，因为文件已经存在，不ftruncate的话mmap最大映射文件实际大小，没有数据就是0B
    ftruncate(fd,fileSize);
    char *pStart=(char*)mmap(NULL,fileSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    ERROR_CHECK(pStart,(char*)-1,"mmap");
    ret=recvCycle(socketFd,pStart,fileSize);//拿数据
    if(0==ret)
    {
        munmap(pStart,fileSize);
        gettimeofday(&end,NULL);
        printf("100.00%s\n","%");
        printf("use time=%ld\n",(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
    }else{
        printf("error load\n");
    } */
    //splice接收,成功啦，接收569M用时22s，mmap用29s
     int rcPipeFd[2];
    pipe(rcPipeFd);//初始化无名管道
    while(total<fileSize){//splice的一端必须为管道，2020-4-1试过
        recvNum=splice(socketFd,NULL,rcPipeFd[1],NULL,fileSize-total,SPLICE_F_MORE|SPLICE_F_MOVE);
        ERROR_CHECK(recvNum,-1,"splice");//内核缓冲区大小1G所以一次性最大接收65536B
        sendToFile=splice(rcPipeFd[0],NULL,fd,NULL,recvNum,SPLICE_F_MORE|SPLICE_F_MOVE);
        ERROR_CHECK(sendToFile,-1,"splice");
        if(recvNum!=sendToFile){//管道错误
            goto end;
        }
        downLoadSize+=sendToFile;
        if(downLoadSize-lastLoadSize>=fileSlice)//当累计下载的文件大小大于自定义的分片大小就会打印进度，才会改变lastLoadsize
            {
                printf("%5.2f%s\r",(double)downLoadSize/fileSize*100,"%");
                fflush(stdout);
                lastLoadSize=downLoadSize;
            }
        total=downLoadSize;
        }
            printf("100.00%s\n","%");
        end:
            gettimeofday(&end,NULL);           
            printf("use time=%lf s\n",(double)((end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec)/1000000);
           // break;   
           //system("clear");
           return 0;
}
