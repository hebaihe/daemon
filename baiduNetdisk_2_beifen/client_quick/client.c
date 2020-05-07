#include "client.h"
train_t taskTrain;//每次发送任务以小火车形式
char password[128]={0};
int main(int argc, char *argv[])
{
    ARGS_CHECK(argc, 3);
    int socketFd, ret;

    //socket-connect
    client_init(&socketFd, argv[1], argv[2]);
    //使用epoll监控标准输入和socketFd
    char buf[1000];
    int epfd = epoll_create(1);
    struct epoll_event event, *evs;
    evs = (struct epoll_event *)calloc(2, sizeof(struct epoll_event));
    event.events = EPOLLIN;
    event.data.fd = socketFd;//添加对socketFd的监听
    epoll_ctl(epfd, EPOLL_CTL_ADD, socketFd, &event);
    event.data.fd = STDIN_FILENO;//添加对标准输入的监听
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &event); 
    ERROR_CHECK(ret, -1, "epoll_ctl");
    int readyFdNum, j;
     syslog(LOG_INFO,"wo hen shuai !!!"); 
    while (1)
    {
        readyFdNum = epoll_wait(epfd, evs, 2, -1); //-1是无限等待
        for (j = 0; j < readyFdNum; j++)
        {
            if (evs[j].data.fd == STDIN_FILENO)
            {
                int len;
                //train_t train;
                bzero(buf, sizeof(buf));
                //bzero(&train, sizeof(train_t));
                bzero(&taskTrain,sizeof(train_t));
                read(STDIN_FILENO, buf, sizeof(buf));
                if (strncmp(buf, "cd", 2) == 0 || strncmp(buf, "pwd", 3) == 0 || strncmp(buf, "download", 8) == 0 || strncmp(buf, "ls", 2) == 0 || strncmp(buf, "remove", 6) == 0 || strncmp(buf, "over", 4) == 0)
                {   
                    len=strlen(buf)-1;//send发送过去默认加\n，所以要把发过去的len-1不发送\n
                    printf("len=%d\n",len);
                    send(socketFd,&len,4,0);
                    send(socketFd,buf,len,0);
                    printf("buf=%s\n",buf);
                    //taskTrain.dataLen=strlen(buf)-1;//规范是要转换大小端
                    //strcpy(taskTrain.buf,buf);
                   // send(socketFd,&taskTrain,4+taskTrain.dataLen,0);
                    //system("clear"); //发送成功后清屏，接收时候不清屏
                }
                else if (strncmp(buf, "upload", 6) == 0)
                {
                     ret = check_dir(socketFd, buf); //检查当前目录是否有待上传的file,有则直接trans_file
                }else if(strncmp(buf, "username", 8) == 0){//!!!!!!!!!!!!如果有时间可以用goto与语句，登录验证时对方发过来type=7跳转到login，然后就可以不用鸡肋的输入username luke了
                   //login:     
                    char username[32];
                    bzero(password,128);
                    sscanf(buf,"%*s%*s%*s%s",password);
                    printf("password=%s\n",password);
                    sscanf(buf,"%*s%s",username);
                    bzero(buf,1000);
                    sprintf(buf,"%s %s","username",username);
                    printf("fenge hou send:%s\n",buf);
                    len=strlen(buf);//此处buf不含\n不需要len-1
                    printf("len=%d\n",len);
                    send(socketFd,&len,4,0);
                    send(socketFd,buf,len,0);
                    printf("buf=%s\n",buf);

                }
                else
                {
                    break;
                }
            }
            if (evs[j].data.fd == socketFd)
            {
                //printf("i am client socketFd\n");
                int dataLen = 0, type = 0;
                recvCycle(socketFd, &type, 4);
                if (type == 4)
                {   
                    printf("child download...\n");
                    recv_file(socketFd);
                    printf("download success");
                }else if(type==7){//密码验证专用，成功输入用户名，得到对方发回的盐值，然后crypt(password,salt)生成密文发给server
                    //goto login;
                    check_salt(socketFd,password);
                }
                else//其他，待扩展
                {
                    bzero(buf, 1000);
                    ret = recvCycle(socketFd, &dataLen, 4);
                    printf("dataLen=%d\n",dataLen);
                    if (ret == -1)
                    {
                        goto end;
                    }
                    if (0 == dataLen)
                    {
                        printf("recv over\n"); //给client发dataLen=0，此处不能break，不然就会结束epoll
                        continue;
                    }
                    recvCycle(socketFd, &buf, dataLen);
                    printf("%s\n", buf);
                }
            }
        }
    }
end:
    close(socketFd);
}
