#include <func.h>
typedef struct{
    int dataLen;//存储的是 buf上到底运了多少数据
    char buf[100];//存储要发送的数据
    int type;//存储任务类型
}train_t,*ptrain_t;
int recvCycle(int newFd,void *pStart,int len);
int client_init(int *socketFd,char *ip,char *port);//socket-connect
int trans_file(int newFd,char *buf);//发送大文件
int recv_file(int socketFd);//接收大文件
int judge(char *buf,ptrain_t train);//判断发送的任务类型，并组装火车 ，这个功能没有用不用看 
int check_dir(int socketFd,char* buf);//检查当前目录是否有待上传的file(file!=buf),有则直接trans_file
int check_salt(int socketFd,char* password);//密码验证专用，成功输入用户名，得到对方发回的盐值，然后crypt(password,salt)生成密文发给server