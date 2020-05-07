#include "process_pool.h"
int do_cd(int newFd,char *task);//执行client的cd命令
int do_pwd(int newFd);
int do_ls(int newFd);
int do_download(int newFd,char *task);
int do_upload(int newFd,char *task);
int do_remove(int newFd,char* task);
int do_check_pwd(int newFd,char* logUsername);//登陆验证
int len,type,ret;
int record_log(int logFd,char* logUsername,char* operation);//日志记录
train_t taskTrain;
char buf[1000];//接任务
char tmp[256];//组合字符串用
char log[4096];//打印日志专用
mmysql_init(conn);
//创建多个子进程
int make_child(process_data_t* p,int processNum)
{
    int sockFds[2];
    int i;
    pid_t pid;
    for(i=0;i<processNum;i++)
    {
        int ret=socketpair(AF_LOCAL,SOCK_STREAM,0,sockFds);
        ERROR_CHECK(ret,-1,"socketpair");
        pid=fork();
        if(0==pid)
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
{//初始化数据库和当前用户
    int newFd,ret;
    char finishFlag=0;
    int logFd=open("server.log",O_RDWR|O_CREAT,0666);//打开日志文件
    char logUsername[128]={0};
    char operation[128]={0};
    ERROR_CHECK(logFd,-1,"open");
    while(1)
    {
        //拿到父进程分配的任务
        recvFd(fds,&newFd);
        if(-1==newFd){//子进程收到主进程发来的newFd=-1代表要求子进程退出
            exit(0);//子进程要退出了
        }
        printf("I am child,get %d\n",newFd);

//设置发卡：登录验证
        int count=0;
        while(1){
            printf("I am login check\n");
            bzero(buf,1000);
            strcpy(buf,"Please inpute your username and password");
            len=strlen(buf);
            type=1000;//7为特定为进行client成功输入username后server发给他salt，然后它输入密码组成密文发回给server
            send(newFd,&type,4,0);
            send(newFd,&len,4,0);
            send(newFd,buf,len,0);
            bzero(logUsername,128);
            ret=do_check_pwd(newFd,logUsername);
            if(ret==0){
                printf("client login success\n");
                record_log(logFd,logUsername,"login success");
                break;
                }else if(ret==-1){
                    printf("client login fail\n");
                    record_log(logFd,logUsername,"login fail");
                    count++;
                    if(count==3){
                        printf("client has no chance\n");
                        goto end;
                    }
                }
        }
        char *a=getcwd(NULL,0);
         len=strlen(a);
         type=6;//pwd为6号命令
        printf("len=%d\n",len);
        send(newFd,&type,4,0);
        send(newFd,&len,4,0);
        send(newFd,a,len,0);
        printf("%s\n",a);
        while(1){//循环接收欧父进程发过来的关于client的任务
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
                record_log(logFd,logUsername,buf);//buf就是client发过来的整个命令字符串，直接存到日志即可
                do_pwd(newFd);
           }else if(strncmp(buf,"ls",2)==0){
                    printf("ls\n");
                    record_log(logFd,logUsername,buf);//此操作就一个参数
                    do_ls(newFd);
           }else if(strncmp(buf,"cd",2)==0){
                printf("cd\n");
                record_log(logFd,logUsername,buf);
                do_cd(newFd,buf);
           }else if(strncmp(buf,"download",8)==0){
                printf("download\n");
                record_log(logFd,logUsername,buf);
                do_download(newFd,buf);
           }else if(strncmp(buf,"upload",6)==0){
                printf("upload\n");
                record_log(logFd,logUsername,buf);
                do_upload(newFd,buf);
           }else if(strncmp(buf,"remove",6)==0){
                printf("remove\n");
                record_log(logFd,logUsername,buf);
                do_remove(newFd,buf);
           }else if(strncmp(buf,"over",4)==0){
               printf("over\n");
               record_log(logFd,logUsername,buf);
               //record_log(logFd,logUsername,"over");
               goto end;
           }
        }
       end:
        close(newFd);
        printf("finish task\n");

        write(fds,&finishFlag,1);//通知父进程我完成任务了

    }
    close(logFd);
}

int do_cd(int newFd,char *task){
    char *dir=(char*)malloc(128);
    memset(dir,0,128);
	sscanf(task,"%*s%s",dir);//dir取空格后的字符串
    printf("get task:%s\n",task);
    printf("get dir:%s\n",dir);
    //bzero(tmp,256);
    //sprintf(tmp,"%s%s","cd",task);
    //record_log(logFd,logUsername,tmp);
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
   // record_log(logFd,logUsername,"pwd");//此操作就一个参数
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
  //  record_log(logFd,logUsername,"ls");//此操作就一个参数
    DIR* pdir = opendir("./");//打开当前目录
    //char buf[1000]={0};
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

void get_salt(char *salt,char *passwd)
{
 int i,j;
 //取出 salt,i 记录密码字符下标,j 记录$出现次数
 for(i=0,j=0;passwd[i] && j != 3;++i)
 {
 if(passwd[i] == '$')
 ++j;
 }
 strncpy(salt,passwd,i-1);
}
//client发来username，我发给他salt，client发来密文，我验证，成功返回0和登录用户名，client故意断开或验证失败，我返回-1
int do_check_pwd(int newFd,char* logUsername){
    
    //收到username,get salt
    char salt[512]={0};
    struct spwd *sp;
    bzero(buf,1000);
    recvCycle(newFd,&len,4);
    printf("len=%d\n",len);
    recvCycle(newFd,buf,len);
    printf("%s\n",buf);
    char userName[100]={0};
	
    sscanf(buf,"%*s%s",userName);//userName取空格后的字符串
    printf("get username:%s\n",userName);
    if((sp=getspnam(userName)) == NULL){
        //perror("getspnam");
        strcpy(logUsername,userName);
        return -1;
    }
    //sp=getspnam(userName);
    char pwd[1024]={0};
    bzero(pwd,1024);
    //char *query_ret=query(conn,q,2);
    get_salt(salt,sp->sp_pwdp);
    printf("salt=%s\n",salt);
    type=7;             //登录验证是第七类型任务
    len=strlen(salt);
    send(newFd,&type,4,0);
    send(newFd,&len,4,0);
    send(newFd,salt,len,0);
    recvCycle(newFd,&len,4);
	recvCycle(newFd,pwd,len);
    printf("client pwd:%s\n",pwd);
    printf("server pwd:%s\n",sp->sp_pwdp);
    if(!strcmp(sp->sp_pwdp,pwd))	
	{   strcpy(logUsername,userName);//将登陆成功的用户名拷贝到需要写日志的用户名
		printf("login success\n");
        len=strlen("login success");
        type=1000;//1000不会进行特殊处理
        send(newFd,&type,4,0);
        send(newFd,&len,4,0);
        send(newFd,"login success",len,0);
	}else{                      
        //也可以把错误登录的用户名记录到日志，将logUsername设为传出参数
        strcpy(logUsername,userName);
		printf("login fail\n");
        return -1;
    }	
    return 0;
}

int record_log(int logFd,char* logUsername,char* operation){//日志记录到server端：用户名+客户端登录及操作+时间
    char tmp[1000]={0};
    //百度查一下获取时间的三种方式
    time_t curtime=0;
    time(&curtime);
    //百度查一下如何将字符串添加到文件末尾（基于文件描述符fd的操作）：open+lseek+write
    
    sprintf(tmp,"%-10s%-30s%-s\n",logUsername,operation,ctime(&curtime));
    printf("tmp=%s\n",tmp);
    lseek(logFd,0,SEEK_END);
    write(logFd,tmp,strlen(tmp));
    //lseek(logFd,0,SEEK_SET);
    //bzero(log,4096);
   //ret= read(logFd,log,sizeof(log));
   // printf("日志记录%d B：\n",ret);
   // printf("%s\n",log);
    //printf("tmp=%s\n",tmp);
    return 0;
}