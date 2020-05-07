#include "process_pool.h"
//#include <mysql/mysql.h>
void sigPipeFunc111(int signum)
{
    signum = 0;
    printf("woshi 10\n");
}
//===========================处理client任务

int do_cd(int newFd, char *task, pusr_info puser); //执行client的cd命令
int do_pwd(int newFd, pusr_info puser);
int do_ls(int newFd, pusr_info puser);
int do_download(int newFd, char *task, pusr_info puser);
int do_upload(int newFd, char *task, pusr_info puser, char *tmp);
int do_remove(int newFd, char *task, pusr_info puser);
int do_check_pwd(int newFd, char *logUsername, pusr_info user); //登陆验证
int do_mkdir(int newFd, char *task, pusr_info puser);
int record_log(int logFd, char *logUsername, char *operation);

//===========================数据库交互函数
//部分自创的sql接口为了与库函数区分开头为he
void hemysql_init();
void heusr_init(pusr_info puser, char *username, int id); //用户信息初始化,user必须为传出参数,将前面成功登陆的用户赋值给user
int query_salt_pwd(char *username, char *salt, char *pwd, int *id);
int query_cd(pusr_info puser, char *dir);
void query_cd_direntname(pusr_info puser); //一次查询不能得出上一级id对应的目录名，所以需要两次查询
int query_ls(pusr_info puser, char *buf);
int remove_file(pusr_info puser, char *task); //query.c
int download_check_file(pusr_info puser, char *file, char *md5);
int upload_check_md5(pusr_info puser, char *md5, char *file); //秒传：找file表-insert dirent-update file
int upload_insert_newfile(pusr_info puser, char *md5, char *file, off_t fileSize);
int insert_mkdir(pusr_info puser, char *dir);

//==============================全局变量
int len, type, ret;
MYSQL *conn;
MYSQL_RES *res;
MYSQL_ROW row;
unsigned int t;
train_t taskTrain;
char buf[1000]; //接任务
char tmp[256];  //组合字符串用
//char log[4096];//打印日志专用
char md5[128];
char file[100]; //接受文件专用
off_t fileSize; //接收文件大小专用

//---------------------------------------------------------------------------------------------

//创建多个子进程
int make_child(process_data_t *p, int processNum)
{
    hemysql_init(); //连接数据库socket+connect,放在这里可以实现只连接一次数据库
    int sockFds[2];
    int i;
    pid_t pid;
    for (i = 0; i < processNum; i++)
    {
        int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, sockFds);
        ERROR_CHECK(ret, -1, "socketpair");
        pid = fork();
        if (0 == pid)
        {
            close(sockFds[0]);
            child_handle(sockFds[1]);
            exit(0);
        }
        close(sockFds[1]);
        //只有父进程在走for循环
        p[i].pid = pid;
        p[i].fd = sockFds[0];
        p[i].busy = 0;
    }
    return 0;
}

int child_handle(int fds)
{

    int newFd, ret;
    char finishFlag = 0;
    int logFd = open("server.log", O_RDWR | O_CREAT, 0666); //打开日志文件
    char logUsername[128] = {0};
    // char operation[128]={0};
    ERROR_CHECK(logFd, -1, "open");
    while (1)
    {
        //拿到父进程分配的任务
        recvFd(fds, &newFd);
        if (-1 == newFd)
        {            //子进程收到主进程发来的newFd=-1代表要求子进程退出
            exit(0); //子进程要退出了
        }
        printf("I am child,get %d\n", newFd);

        //设置发卡：登录验证
        int count = 0;
        //pusr_info puser;//每一个子进程服务一个用户,子进程又挂啦，指针4B你把一个结构体大小的字节置0，访问越界了
        //bzero(puser,sizeof(usr_info));
        pusr_info puser = (pusr_info)calloc(1, sizeof(usr_info));
        //pusr_info precoveryUser=(pusr_info)calloc(1,sizeof(usr_info));
        while (1)
        {
            signal(SIGPIPE, sigPipeFunc111);
            printf("I am login check\n");
            ret=sendTrain(newFd,1000,"Please inpute your username and password");//将三个send封装成小火车
            ERROR_CHECK(ret,-1,"send");
            /*  bzero(buf, 1000);
            strcpy(buf, "Please inpute your username and password");
            len = strlen(buf);
            type = 1000; //7为特定为进行client成功输入username后server发给他salt，然后它输入密码组成密文发回给server
            send(newFd, &type, 4, 0);
            send(newFd, &len, 4, 0);
            send(newFd, buf, len, 0);
           */ bzero(logUsername, 128);
            ret = do_check_pwd(newFd, logUsername, puser);
            if (ret == 0)
            {
                printf("client login success\n");
                printf("username:%s user id:%d user dir:%s user path:%s\n", puser->name, puser->usr_id, puser->dirent, puser->path);
                record_log(logFd, logUsername, "login success");

                break;
            }
            else if (ret == -1)
            {
                printf("client login fail\n");
                record_log(logFd, logUsername, "login fail");
                count++;
                if (count == 3)
                {
                    printf("client has no chance\n");
                    goto end;
                }
            }
        }
        do_pwd(newFd, puser);
        char *a = getcwd(NULL, 0);
        printf("%s\n", a);
        while (1)
        { //循环接收欧父进程发过来的关于client的任务
            bzero(buf, 1000);
            ret = recvCycle(newFd, &len, 4);
            printf("len=%d\n", len);
            if (ret == -1)
            {
                goto end;
            }
            recvCycle(newFd, &buf, len);
            printf("server get task:%s\n", buf);
            //ret=recv(newFd,buf,1000,0);//会造成粘包
            if (strncmp(buf, "pwd", 3) == 0)
            {
                record_log(logFd, logUsername, buf); //buf就是client发过来的整个命令字符串，直接存到日志即可
                do_pwd(newFd, puser);
            }
            else if (strncmp(buf, "ls", 2) == 0)
            {
                printf("ls\n");
                record_log(logFd, logUsername, buf); //此操作就一个参数
                do_ls(newFd, puser);
            }
            else if (strncmp(buf, "cd", 2) == 0)
            {
                printf("cd\n");
                record_log(logFd, logUsername, buf);
                do_cd(newFd, buf, puser);
            }
            else if (strncmp(buf, "download", 8) == 0)
            {
                printf("download\n");
                record_log(logFd, logUsername, buf);
                do_download(newFd, buf, puser);
            }
            else if (strncmp(buf, "upload", 6) == 0)
            {
                printf("upload\n");
                do_upload(newFd, buf, puser, tmp);
                record_log(logFd, logUsername, tmp); //buf三个参数需要拆解，放到do_upload记录日志
            }
            else if (strncmp(buf, "remove", 6) == 0)
            {
                printf("remove\n");
                record_log(logFd, logUsername, buf);
                do_remove(newFd, buf, puser);
            }
            else if (strncmp(buf, "mkdir", 5) == 0)
            {
                printf("mkdir\n");
                record_log(logFd, logUsername, buf);
                do_mkdir(newFd, buf, puser);
            }
            else if (strncmp(buf, "over", 4) == 0)
            {
                printf("over\n");
                record_log(logFd, logUsername, buf);
                goto end;
            }
        }
    end:
        close(newFd);
        printf("finish task\n");

        write(fds, &finishFlag, 1); //通知父进程我完成任务了
    }
    close(logFd);
}
int do_mkdir(int newFd, char *task, pusr_info puser)
{
    char *dir = (char *)calloc(128, sizeof(char));
    sscanf(task, "%*s%s", dir); //dir取空格后的字符串
    printf("get task:%s\n", task);
    printf("get dir:%s\n", dir);
    ret = insert_mkdir(puser, dir);
    if (-2 == ret)
    {
        printf("same dir\n");
        sendTrain(newFd,100,"same dir");//错误类型
       /* bzero(buf, 1000);
        strcpy(buf, "same dir");
        len = strlen(buf);
        type = 100; //错误类型
        send(newFd, &type, 4, 0);
        send(newFd, &len, 4, 0);
        send(newFd, buf, len, 0);*/
    }
    if (-1 == ret)
    {
        printf("sql error\n");
        return -1;
    }
    sendTrain(newFd,12,"mkdir success");
    /*type = 12; //mkdir 类型12
    bzero(buf, 1000);
    strcpy(buf, "mkdir success");
    len = strlen(buf);
    send(newFd, &type, 4, 0);
    send(newFd, &len, 4, 0);
    send(newFd, buf, len, 0);*/
    return 0;
}
int do_cd(int newFd, char *task, pusr_info puser)
{
    char *dir = (char *)calloc(128, sizeof(char));
    sscanf(task, "%*s%s", dir); //dir取空格后的字符串
    printf("get task:%s\n", task);
    printf("get dir:%s\n", dir);
    //查看此用户当前目录下有无待打开的目录：-1没有 0有向打开下一级 1有是..回退上一级
    if (strcmp(dir, ".") == 0)
    {
        printf("bu bian\n");
        return 0;
    }
    ret = query_cd(puser, dir); //改变puser的目录id目录名 path
    if (ret == -1)
    {
        printf("bucunzai\n");
        return 0;
    }
    else if (ret == 0)
    {
        printf("tiaochu query success\n");
    }
    sendTrain(newFd,1,puser->path);
    /*len = strlen(puser->path);
    printf("len=%d\n", len);
    type = 1; //cd为1号命令
    send(newFd, &type, 4, 0);
    send(newFd, &len, 4, 0);
    send(newFd, puser->path, len, 0);*/
    return 0;
}

int do_pwd(int newFd, pusr_info puser)
{
    printf("wojin pwd\n");
    printf("%s\n", puser->path);
    sendTrain(newFd,6,puser->path);
    /*len = strlen(puser->path);
    type = 6;
    send(newFd, &type, 4, 0);
    send(newFd, &len, 4, 0);
    send(newFd, puser->path, len, 0);
   */ return 0;
}

int do_ls(int newFd, pusr_info puser)
{
    bzero(buf, 1000);
    printf("wojin ls\n");
    ret = query_ls(puser, buf);
    //train_t dataTrain;
    if (ret == -1)
    { //0为空，1位有内容，-1为查询错误
        return 0;
    }
    else if (strlen(buf) == 0)
    {
        sendTrain(newFd,100,"dir is empty");
       /* bzero(buf, sizeof(buf));
        strcpy(buf, "dir is empty");
        len = strlen(buf);
        type = 100; //出错类型为100
        send(newFd, &type, 4, 0);
        send(newFd, &len, 4, 0);
        send(newFd, buf, len, 0);*/
    }
    else
    {
        ret=sendTrain(newFd,2,buf);
        ERROR_CHECK(ret,-1,"send");
        /*
        bzero(&dataTrain, sizeof(train_t)); //不用火车发送会在client出现粘包问题
        dataTrain.dataLen = strlen(buf);
        strcpy(dataTrain.buf, buf);
        printf("len=%d buf=%s\n", dataTrain.dataLen, buf);
        type = 2; //ls为2号命令
        send(newFd, &type, 4, 0);
        int ret = send(newFd, &dataTrain, 4 + dataTrain.dataLen, 0);
        ERROR_CHECK(ret, -1, "send");*/
    }

    return 0;
}
int do_download(int newFd, char *task, pusr_info puser)
{
    bzero(file, 100);
    sscanf(task, "%*s%s", file); //dir取空格后的字符串
    printf("get file:%s\n", file);
    ret = download_check_file(puser, file, md5); //检查此用户在当前目录有无这个文件名
    if (ret == -1)
    {
        sendTrain(newFd,100,"download fail");
        /*bzero(buf, 1000);
        strcpy(buf, "download fail");
        len = strlen(buf);
        type = 100; //出错类型为100,目的是不仅如此type=4，4专为下载设的通道
        send(newFd, &type, 4, 0);
        send(newFd, &len, 4, 0);
        send(newFd, buf, len, 0);*/
        return -1;
    }
    else
    {                              //虚拟文件系统存在再在server本地找
        DIR *pdir = opendir("./"); //打开当前目录，这里不用改，因为实际所有用户下载上传的文件都放在当前目录
        if (pdir == NULL)          //下面的判断也都是必须的，查找当前服务器有没有client要求下载的文件
        {
            sendTrain(newFd,100,"open fail");
           /* len = 9;
            type = 100; //出错类型为100,目的是不仅如此type=4，4专为下载设的通道
            send(newFd, &type, 4, 0);
            send(newFd, &len, 4, 0);
            send(newFd, "open fail", 9, 0);*/
        }
        else
        {
            struct dirent *mydir;                   //创建目录结构体
            while ((mydir = readdir(pdir)) != NULL) //每次readdir后指针会自动偏移到下一个目录
            {
                if (strncmp(mydir->d_name, ".", 1) == 0 || strncmp(mydir->d_name, "..", 2) == 0)
                {
                    continue;
                }
                if (strcmp(mydir->d_name, md5) == 0) //server以MD5命名
                {
                    //len = strlen(file);//client接收的文件名依然是在他目录查出来的文件名
                    type = 4;
                    send(newFd, &type, 4, 0);
                    printf("filename:%s md5:%s\n", file, md5);
                    trans_file(newFd, file, md5); //send filename-size-nei rong
                }
            }
        }
    }

    return 0;
}
//先不进行大小+md5的判断,
int do_upload(int newFd, char *task, pusr_info puser, char *tmp) //task= upload file md5 最后一个参数用来去掉MD5然后把任务传出写日志用
{
    bzero(file, 100);
    bzero(md5, 128);
    bzero(tmp, 256);
    sscanf(task, "%*s%s %s", file, md5); //file取空格后的字符串
    printf("get file:%s get md5:%s\n", file, md5);
    sprintf(tmp, "%s %s", "upload", file);
    ret = upload_check_md5(puser, md5, file); //返回0能秒传证明file表有记录，直接传出size就能实现获取size插到dirent
    if (-2 == ret)
    {
        printf("same file\n");
        sendTrain(newFd,100,"same file");
        // bzero(buf, 1000);
        // strcpy(buf, "same file");
        // len = strlen(buf);
        // type = 100; //错误类型
        // send(newFd, &type, 4, 0);
        // send(newFd, &len, 4, 0);
        // send(newFd, buf, len, 0);
    }
    if (-1 == ret)
    {
        printf("zhunbei uploading...\n");
        sendTrain(newFd,3,file);
        /*type = 3;
        len = strlen(file);
        send(newFd, &type, 4, 0);
        send(newFd, &len, 4, 0);
        send(newFd, file, len, 0);*/
        recv_file(newFd, md5, &fileSize);
        ret = upload_insert_newfile(puser, md5, file, fileSize);
        if (ret == -1)
        {
            printf("sql fail\n");
            return -1;
        }
        printf("upload success\n");
        sendTrain(newFd,1000,"upload success");
       /* bzero(buf, 1000);
        strcpy(buf, "upload success");
        len = strlen(buf);
        type = 1000; //upload type为3
        send(newFd, &type, 4, 0);
        send(newFd, &len, 4, 0);
        send(newFd, buf, len, 0);*/
    }
    else if (0 == ret)
    {
        printf("quickly upload success\n");
        sendTrain(newFd,1000,"quickly upload success");
        // bzero(buf, 1000);
        // strcpy(buf, "quickly upload success");
        // len = strlen(buf);
        // type = 1000; //不需要recv_file直接通知对方上传成功
        // send(newFd, &type, 4, 0);
        // send(newFd, &len, 4, 0);
        // send(newFd, buf, len, 0);
    }
    return 0;
}

int do_remove(int newFd, char *task, pusr_info puser)
{
    bzero(file, 100);
    sscanf(task, "%*s%s", file); //dir取空格后的字符串
    printf("get file:%s\n", file);
    ret = remove_file(puser, file);
    ERROR_CHECK(ret, -1, "remove");
    sendTrain(newFd,5,"remove success");
    // len = 14;
    // type = 5; //upload type为3
    // send(newFd, &type, 4, 0);
    // send(newFd, &len, 4, 0);
    // send(newFd, "remove success", len, 0);
    return 0;
}

//client发来username，我发给他salt，client发来密文，我验证，成功返回0和登录用户名，client故意断开或验证失败，我返回-1
int do_check_pwd(int newFd, char *logUsername, pusr_info puser)
{
    //收到username,get salt
    char salt[100] = {0};
    char pwd[256] = {0}; //接收client发来的密文
    bzero(salt, 100);
    bzero(pwd, 256);
    //struct spwd *sp;
    bzero(buf, 1000);
    recvCycle(newFd, &len, 4);
    printf("len=%d\n", len);
    recvCycle(newFd, buf, len);
    printf("%s\n", buf);
    char userName[100] = {0};

    sscanf(buf, "%*s%s", userName); //userName取空格后的字符串
    printf("get username:%s\n", userName);
    int id = 0;
    ret = query_salt_pwd(userName, salt, pwd, &id);
    if (-1 == ret)
    {
        printf("cuole\n");
        strcpy(logUsername, userName);
        return -1;
    }
    printf("成功从数据库获取\n");
    printf("salt=%s\n", salt);
    char serPwd[256] = {0};
    strcpy(serPwd, pwd);
    len = strlen(serPwd);
    printf("server len:%d\n", len);
    printf("server pwd:%s\n", serPwd);
    printf("server id:%d\n", id);
    //登录验证是第七类型任务（+第14类型）
    len = strlen(salt);
    if (0 == len)
    { //并没有此用户对应的salt，不加判断会子进程报错
    sendTrain(newFd,100,"login fail");
        // len = 10;
        // type = 100;
        // send(newFd, &type, 4, 0);
        // send(newFd, &len, 4, 0);
        // send(newFd, "login fail", len, 0);
        strcpy(logUsername, userName);
        return -1;
    }
    sendTrain(newFd,7,salt);
    // type = 7;
    // send(newFd, &type, 4, 0);
    // send(newFd, &len, 4, 0);
    // send(newFd, salt, len, 0);
    recvCycle(newFd, &len, 4);
    recvCycle(newFd, pwd, len);
    printf("client pwd:%s\n", pwd);
    if (!strcmp(serPwd, pwd) && serPwd[0] != 0 && userName[0] != 0)
    {
        strcpy(logUsername, userName);   //将登陆成功的用户名拷贝到需要写日志的用户名
        heusr_init(puser, userName, id); //user必须为传出参数,将前面成功登陆的用户赋值给user
        printf("login success\n");
        type = 14; //帮助client跳出登录约束的flag
        send(newFd, &type, 4, 0);
    }
    else
    {
        //也可以把错误登录的用户名记录到日志，将logUsername设为传出参数
        strcpy(logUsername, userName);
        printf("login fail\n");
        return -1;
    }
    return 0;
}

int record_log(int logFd, char *logUsername, char *operation)
{ //日志记录到server端：用户名+客户端登录及操作+时间
    char tmp[1000] = {0};
    time_t curtime = 0;
    time(&curtime);
    sprintf(tmp, "%-10s%-30s%-s\n", logUsername, operation, ctime(&curtime));
    printf("tmp=%s\n", tmp);
    lseek(logFd, 0, SEEK_END);
    write(logFd, tmp, strlen(tmp));
    lseek(logFd, 0, SEEK_SET);
    return 0;
}

int sendTrain(int newFd,int type,char* buf)//将三个send封装成小火车
{
    train_t dataTrain;
    dataTrain.type=type;
    dataTrain.dataLen = strlen(buf);
    strcpy(dataTrain.buf, buf); //实现copy自定义大小
    send(newFd, &dataTrain, 8 + dataTrain.dataLen, 0);
    ERROR_CHECK(ret,-1,"send");
    return 0;
}
void heusr_init(pusr_info puser, char *username, int id) //用户信息初始化,user必须为传出参数,将前面成功登陆的用户赋值给user
{
    bzero(puser, sizeof(usr_info));
    puser->usr_id = id;   //用户当前目录为根目录
    puser->dirent_id = 0; //用户最初目录为根目录
    strcpy(puser->dirent, "/");
    strcpy(puser->path, "~");
    strcpy(puser->name, username);
    puser->conn = conn; //此处传过来之后就可以到其他.c文件中继续跟着user进行已连接的数据库访问，巧妙
}
void hemysql_init()
{ //何h

    const char *server = "localhost";
    const char *user = "root"; //root用户对所有数据库都有权限
    const char *password = "l123";
    const char *database = "baiduNetdisk"; //要访问的数据库名称
    printf("wo yao lianjie shujuku\n");
    conn = mysql_init(NULL);                                                     //连接初始化--socket
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) //connect
    {
        printf("Error connecting to database:%s\n", mysql_error(conn));
    }
    else
    {
        printf("Connected...\n");
    }
}
int query_salt_pwd(char *username, char *salt, char *pwd, int *id)//还未登录，无puser传过来conn，不能移走
{ //获取对应用户名的salt和pwd，成功返回0，失败-1
    //MYSQL *conn;//conn相当于与数据库通信的socket描述符
    char query[300] = "select salt,pwd,id from user where name='"; //设置flag增加if，flag为1进入query拼接成查询salt和pwd，哈哈哈
    sprintf(query, "%s%s%s", query, username, "'");
    puts(query);
    t = mysql_query(conn, query); //query增删改查都能发送
    if (t)                        //返回1失败
    {                             //Commands out of sync; you can't run this command now:由于上次错误退出时候未将MYSQL_RES所指对象释放，即在下次查询失败返回-1时要mysql_free_result();
        printf("Error making query:%s\n", mysql_error(conn));
        return -1;
    }
    else
    {
        res = mysql_use_result(conn); //使用时mysql_use_result()，必须执行 mysql_fetch_row()直到NULL返回一个 值，
//否则，未提取的行将作为下一个查询的结果集的一部分返回。Commands out of sync; you can't run this command now如果您忘记这样做，C API会给出错误！
        if (res)  //返回值仅为执行的成功失败：1成功 0失败
        {
            while ((row = mysql_fetch_row(res)) != NULL) //自动跳转到下一行
            {
                printf("num=%d\n", mysql_num_fields(res)); //列数
                bzero(salt, 32);
                bzero(pwd, 256);
                strcpy(salt, row[0]); //---
                strcpy(pwd, row[1]);  //---
                *id = atoi(row[2]);
                printf("sql salt=%s\n", salt);
                printf("sql pwd=%s\n", pwd);
                printf("sql id=%d\n", *id);
                //就算没有查询结果退出时候salt和pwd也是成功以空格退出，所以要加判断
                if (salt[0] == 0 || pwd[0] == 0) //结果为空
                {
                    mysql_free_result(res); //使用完必须释放掉
                    return -1;
                }
                for (t = 0; t < mysql_num_fields(res); t++)
                {

                    printf("%s         ", row[t]); //row代表一整行，row[0]是一行第一个参数，row[1]是一行第二个参数
                }
                printf("\n");
                mysql_free_result(res); //使用完必须释放掉
                return 0;               //只需查一次盐值就返回
            }
        }
        else
        {
            printf("Don't find data\n");
            mysql_free_result(res); //使用完必须释放掉
            return -1;
        }
        mysql_free_result(res); //使用完必须释放掉
    }
    return 0;
}
