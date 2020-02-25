#include"process_pool.h"
/*
void factory_init(pfactory *ppf){

	*ppf=(pfactory)malloc(sizeof(factory));
	(*ppf)->user=(pusr_info)malloc(sizeof(usr_info));	
	bzero((*ppf)->user,sizeof(usr_info));
 	memcpy((*ppf)->user->dirent,"~",1);
 	memcpy((*ppf)->user->path,"~/",2);
	mmysql_init(&((*ppf)->conn));

}
*/
/*我感觉这样子也可以
void factory_init(pfactory pf){

	pfactory pf1=(pfactory)malloc(sizeof(factory));
	pf1->user-(pusr_info)calloc(1,sizeof(usr_info));
	strcpy(pf1->user->dirent,"~");
	strcpy(pf1->user->path,"~/");
	*pf=*pf1;

}*/
/*
void mmysql_init(MYSQL **conn){
	int ret = mmysql_connect(conn);
	if(-1==ret){
		printf("mysql connect failure\n");
		exit(-1);
	 }
 }
 */
 /*
 int mmysql_connect(MYSQL **conn){

	char *server = "localhost";
	char *user="root";
	char *password="l123";
	char *database="baiduNetdisk";
	
	*conn = mysql_init(NULL);
	if(!mysql_real_connect(*conn,server,user,password,database,0,NULL,0)){
		printf("Error connect %s\n",mysql_error(*conn));
		return -1;
	}else{
		printf("connect..\n");
		return 1;
	}
}*/
/*
int mysql_delete(MYSQL *conn,char *q){
	int t;
	t=mysql_query(conn,q);
	if(t){
		printf("Error making query:%s\n",mysql_error(conn));
		return -1;
	}else{
		printf("delete success\n");
	}
}
int update(MYSQL *conn,char *q){

	int t,r;
	t=mysql_query(conn,q);
	if(t){
		printf("Error making query:%s\n",mysql_error(conn));
	return -1;
	}else{
		printf("update success\n");
	}

}

int insert(MYSQL *conn,char *q){
//	MYSQL *conn;
	int t,r;
	t=mysql_query(conn,q);
	if(t){
		 printf("Error making query :%s\n",mysql_error(conn));
		return -1;
	}else{
		printf("insert success\n");
	}
	return 1;
}
void query_1(MYSQL_RES *res){//根据查询结果打印表格
	int t;
	MYSQL_ROW row;
	while((row=mysql_fetch_row(res))!=NULL){//打印每一行，自动偏移到下一行
		for(t=0;t<mysql_num_fields(res);t++){//打印每一列
			printf("%s ",row[t]);
		}
		printf("\n");
	}
}
char *query_2(MYSQL_RES *res){//查询salt专用？
	MYSQL_ROW row;
	row=mysql_fetch_row(res);
	if(NULL==row){
		return NULL;
	}

	int len = strlen(row[3]);
	char *ret = (char*)malloc((13+len)*sizeof(char));
	sprintf(ret,"%s",row[3]);
	return ret;
}
char * query(MYSQL *conn,char *q,int flag){
//	MYSQL *conn;
	
	int r,t;
	char *ret;
	MYSQL_RES *res;
	t=mysql_query(conn,q);//库接口
	if(t){
		printf("Error making query :%s\n",mysql_error(conn));
		flag=1;
		goto end;
	}else{
		res = mysql_use_result(conn);
		if(res){
			if(1==flag){
				query_1(res);
			}else{
				ret = query_2(res);	
			}
		}	

		mysql_free_result(res);
	}//if-t
end:

	if(1==flag){
		return NULL;
	}else{
		return ret;
	}
}

MYSQL_RES *query_res(MYSQL *conn,char *q){
	
//	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	int r,t;
	char *ret;
	t=mysql_query(conn,q);
	if(t){
		printf("Error making query :%s\n",mysql_error(conn));
		goto end;
	}else{
		res = mysql_use_result(conn);
		if(res){
//			mysql_close(conn);
		
			return res;
		}	

	//	mysql_free_result(res);
	}//if-t
end:
	//mysql_close(conn);
	return NULL;

}

*/