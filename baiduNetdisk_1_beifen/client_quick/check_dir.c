#include "client.h"

int check_dir(int socketFd,char* buf)//检查当前目录是否有待上传的file,有则直接trans_file
{
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
   DIR* pdir = opendir("/home/luke/day35/client_quick/");//打开当前目录
	if(pdir == NULL)
	{   
        printf("upload fail\n");
        return -1;
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
				printf("wojinlaile\n");
                printf("%s\n",mydir->d_name);
                printf("uploading...\n");
				taskTrain.dataLen=strlen(buf);//规范是要转换大小端
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

