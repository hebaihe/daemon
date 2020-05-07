//这个宏定义是因为splice不是Posix标准，属于新扩展的功能么
#define _GNU_SOURCE
#include "client.h"

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
    //int fd=open(buf,O_RDWR|O_CREAT|O_EXCL,0666);//文件若不存在则建立该文件，否则将导致打开文件错误
    int fd=open(buf,O_RDWR|O_CREAT,0666);//文件不存在则新建，存在则以可读可写权限打开
//发送已下载文件的大小给server，实现断点续传 
    off_t len;
    struct stat mystat;
    bzero(&mystat, sizeof(stat));
	stat(buf, &mystat);
    //本地没下载完的文件大小会把末尾的\0或\n也算上，所以-1以传送实际的有用数据大小，也需要把lseek 从末尾前一个开始字节写，以实现覆盖掉\0或\n
    //问题知道了：是vim创建的文件会在内容后面多一个\n用vscode创建的文件不会出现这种情况,所以vim编辑的文件需要发给server端len-1然后从SEEK_END -1处开始写入
    len=mystat.st_size;
    printf("len=%ld\n",len);
    bzero(buf,1000);
    read(fd,buf,1000);
    printf("文件内容：%s\n",buf);
    //len=5;//先用hello看能否接收对,因为send默认发\n所以需要len-1以保持接收的时候从\n处开始覆盖
    send(socketFd,&len,4,0);
//接收文件大小
    off_t fileSize,fileSlice,lastLoadSize=0,downLoadSize=0,sendToFile=0,total=0,recvNum;
    recvCycle(socketFd,&dataLen,4);
    recvCycle(socketFd,&fileSize,dataLen);
    fileSlice=fileSize/10000;
//接文件内容
    struct timeval start,end;
    gettimeofday(&start,NULL);
    lseek(fd,0,SEEK_END);//为实现断点续传，从文件末尾开始接数据
/*   //先做出对应大小的文件，空间换时间，发送方不需要，ftruncate，因为文件已经存在，不ftruncate的话mmap最大映射文件实际大小，没有数据就是0B
    ftruncate(fd,fileSize);
    char *pStart=(char*)mmap(NULL,fileSize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    ERROR_CHECK(pStart,(char*)-1,"mmap");
   int ret=recvCycle(socketFd,pStart,fileSize);//拿数据
    if(0==ret)
    {
        munmap(pStart,fileSize);
        gettimeofday(&end,NULL);
        printf("100.00%s\n","%");
        printf("use time=%ld\n",(end.tv_sec-start.tv_sec)*1000000+end.tv_usec-start.tv_usec);
    }else{
        printf("error load\n");
    } 
 */   //splice接收,成功啦，接收569M用时22s，mmap用29s
 
     int rcPipeFd[2];
    pipe(rcPipeFd);//初始化无名管道
    while(total<fileSize){
        recvNum=splice(socketFd,NULL,rcPipeFd[1],NULL,fileSize-total,SPLICE_F_MORE|SPLICE_F_MOVE);
        ERROR_CHECK(recvNum,-1,"splice");//内核缓冲区大小1G所以一次性最大接收65536B
        sendToFile=splice(rcPipeFd[0],NULL,fd,NULL,recvNum,SPLICE_F_MORE|SPLICE_F_MOVE);
        ERROR_CHECK(sendToFile,-1,"splice");
        if(recvNum!=sendToFile){//管道错误
            goto end;
        }
        downLoadSize+=sendToFile;
        if(downLoadSize-lastLoadSize>=fileSlice)
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
