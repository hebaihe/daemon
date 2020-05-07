//1、数据库连接，通过数据库存储用户名，salt 值(采用随机字符串生成)，密码(密文形式存储)，通过数据库存储日志
//如何生成随机字符串，参考下面代码
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<string.h>
#include<crypt.h>
#define STR_LEN 10//定义随机输出的字符串长度。
void get_rand_str(char s[],int num){
	char *str="0123456789ABCDEFGHJKLMNOPQRSTUVWXYZabcdefghjklmnopqrstuvwxyz";
    int i,lstr;
    lstr = strlen(str);
    char ss[2]={0};
    srand(time(NULL));
    for(i=1;i<=num;i++){
    	sprintf(ss,"%c",str[rand()%lstr]);// \0占一个字节
        strcat(s,ss);//添加到末尾
    }
    printf("随机数：%s\n",s);//输出生成的随机数。
}

 char* crypt_pwd(char *password,char* salt){
     char s[13]={0};
     sprintf(s,"%s%s","$6$",salt);
     bzero(salt,sizeof(salt));
     strcpy(salt,s);
     printf("salt=%s\n",salt);
    return crypt(password,salt);
}

int main(){
    printf("i am register\n");
    char salt[13]={0};
    get_rand_str(salt,8);
    char *pwd=crypt_pwd("l123",salt);//先生成13位盐值再生成密文
    printf("pwd=%s\n",pwd);

    return 0;
}