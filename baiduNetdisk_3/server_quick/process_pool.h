#include <func.h>
#include <openssl/md5.h>
#include <mysql/mysql.h>

typedef struct
{
	pid_t pid;  //子进程的pid
	int fd;		//子进程的管道对端
	short busy; //子进程的状态
} process_data_t;

typedef struct
{
	int type;
	int dataLen;	//存储的是 buf上到底运了多少数据
	char buf[1000]; //存储要发送的数据
} train_t;
int make_child(process_data_t *, int);
int child_handle(int);
int tcp_init(int *, char *, char *); //初始化socket，bind，listen
int sendFd(int, int);
int recvFd(int, int *);
int trans_file(int newFd, char *filename, char *md5);
int recv_file(int newFd, char *md5, off_t *outFileSize); //接受文件+传出file大小便于insert
int recvCycle(int newFd, void *pStart, int len);
int sendTrain(int newFd,int type,char* buf);//将三个send封装成小火车
//-----------factory:用户+目录+路径
typedef struct USERINFO
{ //一直跟随着当前用户操作后的信息：用户名+id，目录名+id，路径
	char name[512];
	char dirent[512];
	char path[512]; //当前用户所在路径=pwd
	int usr_id;
	int dirent_id; //当前所在目录id,目录表中形式：preid，实现ls
	MYSQL *conn;
} usr_info, *pusr_info;
/*其实下面的函数和函数都是参考别人的，结果应用到我2期代码根本无法衔接，所以基本上都没用到
typedef struct FACTORY{//查询用户对应信息
	pusr_info user;
	MYSQL *conn;   //（在数据库中查询用）
}factory,*pfactory;

void factory_init(pfactory *ppf);*/
//-----------sql下面的基本没用到
void mmysql_init(MYSQL **conn);
int mmysql_connect(MYSQL **);
int mysql_delete(MYSQL *conn, char *);
int update(MYSQL *, char *);
int insert(MYSQL *, char *);
char *query(MYSQL *, char *, int flag);
void query_1(MYSQL_RES *res);		   //根据查询结果打印表格
char *query_2(MYSQL_RES *res);		   //查询salt专用？
//MYSQL_RES *query_res(MYSQL *, char *); //接收查询结果？