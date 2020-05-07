#include <func.h>

int sendFd(int sfd,int fd)
{
    struct msghdr msg;
    bzero(&msg,sizeof(msg));
    //用户态要给对方什么信息
    struct iovec iov[2];
    char buf[10]="hello";
    iov[0].iov_base=buf;
    iov[0].iov_len=5;
    iov[1].iov_base=&fd;
    iov[1].iov_len=4;
    msg.msg_iov=iov;
    msg.msg_iovlen=2;
    //内核控制信息要传递那个
    struct cmsghdr *cmsg;
    int len=CMSG_LEN(sizeof(int));
    cmsg=(struct cmsghdr *)calloc(1,len);
    cmsg->cmsg_len=len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    *(int*)CMSG_DATA(cmsg)=fd;
    if(-1==fd){
        *(int*)CMSG_DATA(cmsg)=0;
    }
    msg.msg_control=cmsg;
    msg.msg_controllen=len;
    int ret;
    ret=sendmsg(sfd,&msg,0);
    ERROR_CHECK(ret,-1,"sendmsg");
    return 0;
}
int recvFd(int sfd,int *fd)
{
    int exitFlag=0;
    struct msghdr msg;
    bzero(&msg,sizeof(msg));
    //用户态要接什么信息
    struct iovec iov[2];
    char buf[10]={0};
    iov[0].iov_base=buf;
    iov[0].iov_len=5;
    iov[1].iov_base=&exitFlag;
    iov[1].iov_len=4;
    msg.msg_iov=iov;
    msg.msg_iovlen=2;
    //内核控制信息要传递那个
    struct cmsghdr *cmsg;
    int len=CMSG_LEN(sizeof(int));
    cmsg=(struct cmsghdr *)calloc(1,len);
    cmsg->cmsg_len=len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    msg.msg_control=cmsg;
    msg.msg_controllen=len;
    int ret;
    ret=recvmsg(sfd,&msg,0);
    ERROR_CHECK(ret,-1,"recvmsg");
    *fd=*(int*)CMSG_DATA(cmsg);//拿到控制信息在当前进程中的位置
    if(-1==exitFlag){
        *fd=-1;
    }
    return 0;
}

