#include "client.h"
#define _XOPEN_SOURCE
int check_dir(int socketFd,char* buf)//检查当前目录是否有待上传的file,有则直接trans_file
{
		printf("wojin checkdir\n");
	train_t taskTrain;	
	printf("buf=%s\n",buf);
    char *file=(char*)malloc(128);
    memset(file,0,128);
	sscanf(buf,"%*s%s",file);//dir取空格后的字符串
	printf("file=%s\n",file);
	/*printf("uploading...\n");
	send(socketFd,buf,strlen(buf)-1,0);
    trans_file(socketFd,file);
	printf("upload over\n");
	return 0;*/
   // DIR* pdir = opendir("/home/luke/baiduNetdisk_2_beifen/client_quick/");//使用绝对路径打开当前目录
   DIR* pdir = opendir("./");//使用相对路径打开当前目录
	if(pdir == NULL)
	{   
        printf("upload fail\n");
        return -1;
	}else
	{
		printf("wojin checkdir\n");
		struct dirent* mydir ;//创建目录结构体
		while((mydir = readdir(pdir)) != NULL)//每次readdir后指针会自动偏移到下一个目录
		{
			if(strncmp(mydir->d_name, ".", 1) == 0 || strncmp(mydir->d_name,"..", 2) == 0)
			{
				continue ;
			}
            if(strcmp(mydir->d_name,file)==0){
				printf("wojinlaile\n");
                printf("%s\n",mydir->d_name);
                printf("uploading...\n");
				taskTrain.dataLen=strlen(buf)-1;//规范是要转换大小端,send发送过去默认加\n，所以要把发过去的len-1不发送\n
                strcpy(taskTrain.buf,buf);
                send(socketFd,&taskTrain,4+taskTrain.dataLen,0);
                trans_file(socketFd,file);
				printf("upload over\n");
                return 0;
            }
		}
	}
    printf("upload fail\n");
    return -1;
}

int check_salt(int socketFd,char* password)//密码验证专用，成功输入用户名，得到对方发回的盐值，然后crypt(password,salt)生成密文发给server
{
	int len;
	char salt[128]={0};
	recvCycle(socketFd,&len,4);
	recvCycle(socketFd,salt,len);
	char *pwd=crypt(password,salt);
	len=strlen(pwd);//此处buf不含\n不需要len-1
	send(socketFd,&len,4,0);
	send(socketFd,pwd,len,0);
	printf("salt=%s\n",salt);
	printf("len=%d\n",len);
	printf("pwd=%s\n",pwd);
	return 0;
}