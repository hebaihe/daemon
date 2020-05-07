#include <func.h>

int client_init(int *socketFd,char *ip,char *port)
{
    int sfd;
    //初始化一个socket，有一个缓冲区
    sfd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(sfd,-1,"socket");
    printf("socketFd=%d\n",sfd);
    struct sockaddr_in ser;
    bzero(&ser,sizeof(ser));
    ser.sin_family=AF_INET;
    ser.sin_port=htons(atoi(port));//字符串转int转网络字节序
    ser.sin_addr.s_addr=inet_addr(ip);
    int ret;
    ret=connect(sfd,(struct sockaddr*)&ser,sizeof(ser));
    ERROR_CHECK(ret,-1,"connect");
    *socketFd=sfd;
    return 0;
}

