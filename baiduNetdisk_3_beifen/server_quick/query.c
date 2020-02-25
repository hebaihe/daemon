#include "process_pool.h"
int he_delete(MYSQL *conn, char *q);
int he_update(MYSQL *conn, char *q);
int he_update(MYSQL *conn, char *q);
int he_insert(MYSQL *conn, char *q);
MYSQL_RES *query_res(pusr_info puser, char *query);
void query_cd_direntname(pusr_info puser);
MYSQL_RES *res;
MYSQL_ROW row;
char tmp[256];
unsigned int t;
int query_cd(pusr_info puser, char *dir)
{
    if (strcmp(dir, "..") == 0)//确认当前目录的id需要查一次，确认当前目录id的目录名需要再查一次
    {
        printf("puser->dirent_id:%d\n",puser->dirent_id);
        if (puser->dirent_id == 0)
        { //已经是根目录
            printf("已经根目录\n");
            return 0;//成功查询
        }
        else //仅实现除dirent的更新
        {
            char query[300] = "select preid,filename from dirent where id=";
            sprintf(query, "%s%d%s%d", query, puser->dirent_id, " and type='dir' and userid=", puser->usr_id);
            puts(query);
            res=query_res(puser,query);//此行代替了下面9行代码，前提是你的sql语句没有错误
            // t = mysql_query(puser->conn, query); //query增删改查都能发送
            // if (t)
            // {
            //     printf("Error making query:%s\n", mysql_error(puser->conn));
            //     return -1;
            // }
            // else
            // {
            //     res = mysql_use_result(puser->conn); //使用时mysql_use_result()，必须执行 mysql_fetch_row()直到NULL返回一个 值，
                if (res)                      //返回值仅为sql执行的成功失败：1成功 0失败
                {
                    while ((row = mysql_fetch_row(res)) != NULL) //查询一行后自动跳转到下一行，就算没有查询结果也会进入这个循环
                    {
//不是根就要更dir减strlen(filename+1),返回值需要比较特殊来进行第二次查询select filename from dirent where id=puser->dirent_id
                        bzero(puser->dirent, 512);
                        puser->dirent_id = atoi(row[0]);

                        int len = strlen(row[1]) + 1;
                        bzero(tmp, 256);
                        strncpy(tmp, puser->path, strlen(puser->path) - len);
                        bzero(puser->path, 512);
                        strcpy(puser->path, tmp);
                        printf("sql dirent_id=%d\n", puser->dirent_id);
                        printf("path=%s\n", puser->path);
                        
                        mysql_free_result(res);     //使用完必须释放掉
                        if(0==puser->dirent_id){
                            bzero(puser->dirent,512);
                            strcpy(puser->dirent,"~");
                        }else{
                            printf("yici query over\n");
                            query_cd_direntname(puser);//通过目录id确认并传出目录名
                            return 1;                   //1特殊处理：还得sql查当前目录id对应目录名
                        }
                         
                        
                    }
                }
            //}
        }
    }
    else
    {
        char query[300] = "select id,filename from dirent where preid=";
        sprintf(query, "%s%d%s%d%s%s%s", query, puser->dirent_id, " and userid=", puser->usr_id, " and filename='", dir, "' and type='dir'");
        puts(query);
        res=query_res(puser,query);
        // t = mysql_query(puser->conn, query); //query增删改查都能发送
        // if (t)                        //返回1失败
        // {                             //Commands out of sync; you can't run this command now:由于上次错误退出时候未将MYSQL_RES所指对象释放，即在下次查询失败返回-1时要mysql_free_result();
        //     printf("Error making query:%s\n", mysql_error(puser->conn));
        //     return -1;
        // }
        // else
        // {
        //     res = mysql_use_result(puser->conn); //使用时mysql_use_result()，必须执行 mysql_fetch_row()直到NULL返回一个 值，
            // if (res)                      //返回值仅为sql执行的成功失败：1成功 0失败
            // {
                while ((row = mysql_fetch_row(res)) != NULL) //查询一行后自动跳转到下一行，就算没有查询结果也会进入这个循环
                {

                    printf("num=%d\n", mysql_num_fields(res)); //列数
                    bzero(puser->dirent, 512);
                    puser->dirent_id = atoi(row[0]);
                    strcpy(puser->dirent, row[1]);
                    sprintf(puser->path, "%s%c%s", puser->path, '/', dir);
                    printf("sql id=%d\n", puser->dirent_id);
                    printf("puser->dirent=%s\n", puser->dirent);
                    printf("path=%s\n", puser->path);
                    if (row[1][0] == 0) //结果filename为空,不然无法判断查没查到结果
                    {
                        printf("row10=%c\n", row[1][0]);
                        mysql_free_result(res); //使用完必须释放掉
                        return -1;
                    }
                    for (t = 0; t < mysql_num_fields(res); t++)
                    {

                        printf("%s ", row[t]); //row代表一整行，row[0]是一行第一个参数，row[1]是一行第二个参数
                    }
                    printf("\n");
                    mysql_free_result(res); //使用完必须释放掉
                    return 0;               //只需查一次id就返回
                }
            //}
            // else
            // {
            //     printf("Don't find data\n");
            //     mysql_free_result(res); //使用完必须释放掉
            //     return -1;
            // }
            mysql_free_result(res); //使用完必须释放掉
        //}
    }
    return 0;
}
void query_cd_direntname(pusr_info puser)
{

    char query[300] = "select filename from dirent where id=";
    sprintf(query, "%s%d", query, puser->dirent_id);
    puts(query);
    mysql_query(puser->conn, query);
    res = mysql_use_result(puser->conn); //使用时mysql_use_result()，必须执行 mysql_fetch_row()直到NULL返回一个 值，
    row = mysql_fetch_row(res);
    strcpy(puser->dirent, row[0]);
    mysql_free_result(res);
    printf("finally:\n");
    printf("dirid:%d dirent:%s path:%s\n", puser->dirent_id, puser->dirent, puser->path);
}
int query_ls(pusr_info puser, char *buf)
{
    //bzero(buf, 1000); //bzero(buf,sizeof(buf));//报错，传过来的是地址，并不知道buf大小
    char query[300] = "select type,filename,size,ctime from dirent where preid=";
    sprintf(query, "%s%d%s%d", query, puser->dirent_id, " and userid=", puser->usr_id);
    puts(query);
    res=query_res(puser,query);
    // t = mysql_query(puser->conn, query); //query增删改查都能发送
    // if (t)
    // {
    //     printf("Error making query:%s\n", mysql_error(puser->conn));
    //     return -1;
    // }
    // else
    // {
    //     printf("??\n");

    //     res = mysql_use_result(puser->conn); //使用时mysql_use_result()，必须执行 mysql_fetch_row()直到NULL返回一个 值，
        // if (res)                             //返回值仅为sql执行的成功失败：1成功 0失败
        // {
            while ((row = mysql_fetch_row(res)) != NULL) //查询一行后自动跳转到下一行,如果没有查询结果不会进入循环
            {
                printf(">>>>>>>>>\n");
                for (t = 0; t < mysql_num_fields(res); t++)
                {
                    printf("wojin for\n");
                    sprintf(buf, "%s %-15s", buf, row[t]); //不确定row带不带\n
                    //由于查询的结果不止一行，sprintf进行buf组合
                    printf("%s ", row[t]); //row代表一整行，row[0]是一行第一个参数，row[1]是一行第二个参数
                }
                sprintf(buf, "%s%s", buf, "\n");
                printf("buf:%s\n", buf);
            }
        // }
        // else
        // {
        //     printf("Don't find data\n");
        //     mysql_free_result(res); //使用完必须释放掉
        //     return 0;
        // }
    //}
    printf("zuizhong tui\n");
    mysql_free_result(res); //使用完必须释放
    return 1;               //1代表查询到数据
}
int insert_mkdir(pusr_info puser, char *dir)
{
    char query[300] = {0};
    char str[] = " insert into dirent (preid,userid,type,filename) values (";
    char str1[] = "select id from dirent where userid="; //增加判断，当前目录有同名dir则不能上传
    sprintf(query, "%s%d%s%d%s%s%s", str1, puser->usr_id, " and preid=", puser->dirent_id, " and filename='", dir, "'");
    puts(query);
    t = mysql_query(puser->conn, query); //query增删改查都能发送
    if (t)
    {
        printf("Error making query:%s\n", mysql_error(puser->conn));
        return -2;
    }
    else
    {
        res = mysql_use_result(puser->conn);
        if ((row = mysql_fetch_row(res)) != NULL)
        {
            printf("same dir\n");
            mysql_free_result(res);
            return -2;
        }
    }
    sprintf(query, "%s%d%s%d%s%s%s", str, puser->dirent_id, ",", puser->usr_id, ",'dir','", dir, "')");
    puts(query);
    he_insert(puser->conn, query);
    return 0;
}

int download_check_file(pusr_info puser, char *file, char *md5)
{ //传入file，传出MD5
    char query[300] = {0};
    int fileid;
    char str[] = "select fileid from dirent where userid=";
    char str1[] = "select md5str from file where id=";
    sprintf(query, "%s%d%s%d%s%s%s", str, puser->usr_id, " and preid=", puser->dirent_id, " and filename='", file, "'");
    puts(query);
    res=query_res(puser,query);
    // t = mysql_query(puser->conn, query); //query增删改查都能发送
    // if (t)
    // {
    //     printf("Error making query:%s\n", mysql_error(puser->conn));
    //     return -1;
    // }
    // else
    //{
     //   res = mysql_use_result(puser->conn);
        //res=query_res(puser,query);     找时间改成这个，封城函数
        if ((row = mysql_fetch_row(res)) == NULL)
        {
            printf("find fail\n");
            return -1;
        }
        //printf(">>>>>>>>>\n");
        fileid = atoi(row[0]); //获取查询结果第1列
        printf("fileid:%d\n", fileid);
        mysql_free_result(res);
        bzero(query, 300);
        sprintf(query, "%s%d", str1, fileid);
        puts(query);
        res=query_res(puser,query);
        // t = mysql_query(puser->conn, query); //query增删改查都能发送
        // if (t)
        // {
        //     printf("Error making query:%s\n", mysql_error(puser->conn));
        //     return -1;
        // }

        mysql_use_result(puser->conn);
        if ((row = mysql_fetch_row(res)) == NULL)
        {
            printf("find fail\n");
            return -1;
        }
        strcpy(md5, row[0]);
        printf("md5:%s\n", md5);
        mysql_free_result(res);
    //}
    return 0;
}
int remove_file(pusr_info puser, char *task)
{ //除了删除file4的情况都能成功：fileid没有值的问题
    int fileid;
    char query[300] = {0};
    char str[] = "select fileid from dirent where userid=";
    char str1[] = " update file set count=count-1 where id=";
    char str2[] = "delete from dirent where userid=";
    //先找到file表的id
    sprintf(query, "%s%d%s%d%s%s%s", str, puser->usr_id, " and preid=", puser->dirent_id, " and filename='", task, "'");
    puts(query);
    // t = mysql_query(puser->conn, query); //query增删改查都能发送
    // if (t)
    // {
    //     printf("Error making query:%s\n", mysql_error(puser->conn));
    //     return -1;
    // }
    // else
    // {
    //     printf("??\n");
    //     res = mysql_use_result(puser->conn);
        res=query_res(puser,query);     //找时间改成这个，封城函数
        if ((row = mysql_fetch_row(res)) == NULL)
        {
            printf("remove fail\n");
            return -1;
        }
        printf(">>>>>>>>>\n");

        fileid = atoi(row[0]); //获取查询结果第1列
        printf("fileid:%d\n", fileid);
        mysql_free_result(res);
    //}
    //在更新file表的count引用计数
    bzero(query, 300);
    sprintf(query, "%s%d", str1, fileid);
    puts(query);
    he_update(puser->conn, query);
    //再删除dirent对应项
    bzero(query, 300);
    sprintf(query, "%s%d%s%d%s%s%s", str2, puser->usr_id, " and preid=", puser->dirent_id, " and filename='", task, "'");
    he_delete(puser->conn, query);
    return 0;
}
//ret:-2报错 -1：查询失败需要重新上传 0：查询成功秒传
int upload_check_md5(pusr_info puser, char *md5, char *file)
{ //秒传：找file表-insert dirent-update file
    int fileid;
    off_t fileSize;
    char str[] = "select id,size from file where md5str='";
    char str1[] = "update file set count=count+1 where md5str ='";
    char str2[] = "insert into dirent (preid,userid,type,filename,fileid,size) values ("; //可用于秒传和非秒传插入
    char str3[] = "select id from dirent where userid=";                                  //增加判断，当前目录有同名文件则不能上传
    char query[300] = {0};
    //先判断有无同名文件
    sprintf(query, "%s%d%s%d%s%s%s", str3, puser->usr_id, " and preid=", puser->dirent_id, " and filename='", file, "'");
    puts(query);
    // t = mysql_query(puser->conn, query); //query增删改查都能发送
    // if (t)
    // {
    //     printf("Error making query:%s\n", mysql_error(puser->conn));
    //     return -2;
    // }
    // else
    // {
    //     res = mysql_use_result(puser->conn);
    res=query_res(puser,query);
        if ((row = mysql_fetch_row(res)) != NULL)
        {
            printf("same file\n");
            mysql_free_result(res);
            return -2;
        }
    //}
    //没有则查询file表是否已存在同MD5文件
    sprintf(query, "%s%s%s", str, md5, "'");
    puts(query);
    // t = mysql_query(puser->conn, query); //query增删改查都能发送
    // if (t)
    // {
    //     printf("Error making query:%s\n", mysql_error(puser->conn));
    //     return -2;
    // }
    // else
    // {
    //     res = mysql_use_result(puser->conn);
        res=query_res(puser,query);  //找时间改成这个，封城函数
        if ((row = mysql_fetch_row(res)) == NULL)
        {
            printf("find md5 fail\n");
            return -1;
        }                      //查询成功则能秒传证明file表有记录，直接传出size就能实现获取size插到dirent
        fileid = atoi(row[0]); //获取查询结果第1列
        fileSize = atoi(row[1]);
        printf("table file id:%d size:%ld\n", fileid, fileSize);
        mysql_free_result(res);
        bzero(query, 300);
        sprintf(query, "%s%s%s", str1, md5, "'");
        he_update(puser->conn, query);
        bzero(query, 300); //insert into dirent (preid,userid,type,filename,fileid,size) values (3,1,'file','file5',2,666)
        sprintf(query, "%s%d%s%d%s%s%s%d%s%ld%s", str2, puser->dirent_id, ",", puser->usr_id, ",'file','", file, "',", fileid, ",", fileSize, ")");
        puts(query);
        he_insert(puser->conn, query);
    //}
    return 0;
}

int upload_insert_newfile(pusr_info puser, char *md5, char *file, off_t fileSize)
{
    int fileid;
    char str[] = "insert into file (md5str,size,count) values ('";
    char str1[] = "select id from file where md5str='";
    char str2[] = "insert into dirent (preid,userid,type,filename,fileid,size) values ("; //可用于秒传和非秒传插入
    char query[300] = {0};
    sprintf(query, "%s%s%s%ld%s", str, md5, "',", fileSize, ",1)");
    puts(query);
    he_insert(puser->conn, query);
    bzero(query, 300);
    sprintf(query, "%s%s%s", str1, md5, "'");
    puts(query);
    // t = mysql_query(puser->conn, query);
    // if (t)
    // {
    //     printf("Error making query:%s\n", mysql_error(puser->conn));
    //     return -1;
    // }
    // else
    // {
    //     res = mysql_use_result(puser->conn);
        res=query_res(puser,query);  //找时间改成这个，封城函数
        if ((row = mysql_fetch_row(res)) == NULL)
        { //此处不判断了，上一步刚插入肯定能找到
            printf("find md5 fail\n");
            return -1;
        }
        fileid = atoi(row[0]); //获取查询结果第1列
        printf("table file id:%d size:%ld\n", fileid, fileSize);
        mysql_free_result(res);
        bzero(query, 300);
        sprintf(query, "%s%d%s%d%s%s%s%d%s%ld%s", str2, puser->dirent_id, ",", puser->usr_id, ",'file','", file, "',", fileid, ",", fileSize, ")");
        puts(query);
        he_insert(puser->conn, query);
   // }
    return 0;
}

int he_delete(MYSQL *conn, char *q)
{
    int t;
    puts(q);
    t = mysql_query(conn, q);
    if (t)
    {
        printf("Error making query:%s\n", mysql_error(conn));
        return -1;
    }
    else
    {
        printf("delete success\n");
    }
    return 0;
}
int he_update(MYSQL *conn, char *q)
{

    int t;
    puts(q);
    t = mysql_query(conn, q);
    if (t)
    {
        printf("Error making query:%s\n", mysql_error(conn));
        return -1;
    }
    else
    {
        printf("update success\n");
    }
    return 0;
}

int he_insert(MYSQL *conn, char *q)
{
    int t;
    puts(q);
    t = mysql_query(conn, q);
    if (t)
    {
        printf("Error making query :%s\n", mysql_error(conn));
        return -1;
    }
    else
    {
        printf("insert success\n");
    }
    return 0;
}
MYSQL_RES *query_res(pusr_info puser, char *query)
{
    int t;
    t = mysql_query(puser->conn,query);
    if (t)
    {
        printf("Error making query :%s\n", mysql_error(puser->conn));
        return NULL;
    }
    else
    {
        res = mysql_use_result(puser->conn);
        if (res)
        {
            return res;
        }
    }
    return NULL;
}