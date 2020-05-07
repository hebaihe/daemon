#include "process_pool.h"
int do_cd(int,char *);
int do_pwd(int newFd);
int do_ls(int newFd);
int do_download(int newFd,char *task);
int do_upload(int newFd,char *task);
int do_remove(int newFd,char* task);
int len,type;
train_t taskTrain;
//创建多个子进程
int make_child(process_data_t* p,int processNum)
{
    int sockFds[2];
    int i;
    pid_t pid;
    for(i=0;i<processNum;i++)
    {
        int ret=socketpair(AF_LOCAL,SOCK_STREAM,0,sockFds);//在父子进程间创建全双工管道，和无名管道一样只能在亲缘进程间使用
        ERROR_CHECK(ret,-1,"socketpair");
        pid=fork();
        if(0==pid)//进入子进程
        {
            close(sockFds[0]);
            child_handle(sockFds[1]);
            exit(0);
        }
        close(sockFds[1]);
        //只有父进程在走for循环
        p[i].pid=pid;
        p[i].fd=sockFds[0];
        p[i].busy=0;
    }
    return 0;
}

int child_handle(int fds)
{
    int newFd,ret;
    char finishFlag=0;
    char buf[1000];//接任务
    while(1)
    {
        //拿到父进程分配的任务
        recvFd(fds,&newFd);
        if(-1==newFd){//子进程收到主进程发来的newFd=-1代表要求子进程退出
            exit(0);//子进程要退出了
        }
        printf("I am child,get %d\n",newFd);
        //trans_file(newFd,"file");//发送文件给客户端
        //recv_file(newFd);
        char *a=getcwd(NULL,0);
         len=strlen(a);
         type=6;//pwd为6号命令
        printf("len=%d\n",len);
        send(newFd,&type,4,0);
        send(newFd,&len,4,0);
        send(newFd,a,len,0);
        printf("%s\n",a);
        while(1){//循环接收父进程发过来的关于client的任务
            //printf("wo jin lai le\n"); 
                    bzero(buf, 1000);
                    ret = recvCycle(newFd, &len, 4);
                     printf("len=%d\n",len);
                    if (ret == -1)
                    {
                        goto end;
                    }
                    recvCycle(newFd, &buf, len);
                    printf("server get task:%s\n", buf);
             //ret=recv(newFd,buf,1000,0);//会造成粘包
            if(strncmp(buf,"pwd",3)==0){
                do_pwd(newFd);
           }else if(strncmp(buf,"ls",2)==0){
                    printf("ls\n");
                    do_ls(newFd);
           }else if(strncmp(buf,"cd",2)==0){
                printf("cd\n");
                do_cd(newFd,buf);
           }else if(strncmp(buf,"download",8)==0){
                printf("download\n");
                do_download(newFd,buf);
           }else if(strncmp(buf,"upload",6)==0){
                printf("upload\n");
                do_upload(newFd,buf);
           }else if(strncmp(buf,"remove",6)==0){
                printf("remove\n");
                do_remove(newFd,buf);
           }else if(strncmp(buf,"over",4)==0){
               goto end;
           }
        }
       end:
        close(newFd);
        printf("finish task\n");

        write(fds,&finishFlag,1);//通知父进程我完成任务了

    }
}

int do_cd(int newFd,char *task){
    char *dir=(char*)malloc(128);
    memset(dir,0,128);
	sscanf(task,"%*s%s",dir);//dir取空格后的字符串
    printf("get dir:%s\n",dir);
	int ret=chdir(dir);
    ERROR_CHECK(ret,-1,"chdir");
	char *current=getcwd(dir,128);
    printf("%s\n",current);
    len =strlen(current);
    printf("len=%d\n",len);
    type=1;//cd为1号命令
    send(newFd,&type,4,0);
    send(newFd,&len,4,0);
    send(newFd,current,len,0);
    //printf("%s\n",dir);
    //len =strlen(dir);
    //send(newFd,&len,4,0);
    //send(newFd,dir,len,0);
    return 0;
}
int do_pwd(int newFd){
    printf("wojin pwd\n");
    char* a=getcwd(NULL,0);
    printf("%s\n",a);  
    len=strlen(a);
    type=6;
    send(newFd,&type,4,0);
    send(newFd,&len,4,0);
    send(newFd,a,len,0);
    return 0;
}
static char* file_type(mode_t md)
{
	if(S_ISREG(md))
	{
		return "nrml";	
	}else if(S_ISDIR(md))
	{
		return "dir ";
	}else if(S_ISFIFO(md))
	{
		return "pipe";
	}else 
	{
		return "othr" ;
	}
}
int do_ls(int newFd){
    printf("wojin ls\n");
    	DIR* pdir = opendir("./");//打开当前目录
        char buf[1000]={0};
        train_t dataTrain;
	if(pdir == NULL)
	{   
        len=9;
         type=100;//出错类型为100
        send(newFd,&type,4,0);
        send(newFd,&len,4,0);
		send(newFd,"open fail",len,0);
	}else
	{
		struct dirent* mydir ;//创建目录结构体
		while((mydir = readdir(pdir)) != NULL)//每次readdir后指针会自动偏移到下一个目录
		{
			if(strncmp(mydir->d_name, ".", 1) == 0 || strncmp(mydir->d_name,"..", 2) == 0)
			{
				continue ;
			}
			struct stat mystat;
			bzero(&mystat, sizeof(stat));
			stat(mydir->d_name, &mystat);
			bzero(buf,1000);
			sprintf(buf, "%-4s %-20s %10ldB %s",file_type(mystat.st_mode),mydir->d_name, mystat.st_size,ctime(&mystat.st_atime) );
            printf("%s\n",buf);
            bzero(&dataTrain,sizeof(train_t));//不用火车发送会在client出现粘包问题
            dataTrain.dataLen=strlen(buf);
            strcpy(dataTrain.buf,buf);
            type=2;//ls为2号命令
            send(newFd,&type,4,0);
            int ret=send(newFd,&dataTrain,4+dataTrain.dataLen,0);
            ERROR_CHECK(ret,-1,"send");
            //send(newFd,buf,strlen(buf)-1,0);
		}
        //不需要发送文件结束标志的小火车，下载文件才需要给client发
        //dataTrain.dataLen=0;
        //send(newFd,&dataTrain,4,0);
	}

    return 0;
}
int do_download(int newFd,char* task){
     char file[100]={0};
	sscanf(task,"%*s%s",file);//dir取空格后的字符串
    printf("get file:%s\n",file);
    	DIR* pdir = opendir("./");//打开当前目录
	if(pdir == NULL)
	{
        len=9;
        type=100;//出错类型为100,目的是不仅如此type=4，4专为下载设的通道
        send(newFd,&type,4,0);
        send(newFd,&len,4,0);
		send(newFd,"open fail",9,0);
	}else
	{
		struct dirent* mydir ;//创建目录结构体
		while((mydir = readdir(pdir)) != NULL)//每次readdir后指针会自动偏移到下一个目录
		{
			if(strncmp(mydir->d_name, ".", 1) == 0 || strncmp(mydir->d_name,"..", 2) == 0)
			{
				continue ;
			}
            if(strcmp(mydir->d_name,file)==0){
                len=strlen(file);
                type=4;
                send(newFd,&type,4,0);
                trans_file(newFd,file);
            }
		//	sprintf(buf, "%-4s %-20s %10ldB %s",file_type(mystat.st_mode),mydir->d_name, mystat.st_size,ctime(&mystat.st_atime) );

		}
	}
    return 0;
}
int do_upload(int newFd,char *task){
    char file[100]={0};
	sscanf(task,"%*s%s",file);//file取空格后的字符串
    printf("get file:%s\n",file);
    recv_file(newFd);
    printf("upload success\n");
    len=14;
    type=3;//upload type为3
    send(newFd,&type,4,0);
    send(newFd,&len,4,0);
	send(newFd,"upload success",len,0);
    return 0;
}
int do_remove(int newFd,char* task){
    char file[100]={0};
	sscanf(task,"%*s%s",file);//dir取空格后的字符串
    printf("get file:%s\n",file);
    int ret=remove(file);
    ERROR_CHECK(ret,-1,"remove");
    len=14;
    type=5;//upload type为3
    send(newFd,&type,4,0);
    send(newFd,&len,4,0);
	send(newFd,"remove success",len,0);
    return 0;
}
