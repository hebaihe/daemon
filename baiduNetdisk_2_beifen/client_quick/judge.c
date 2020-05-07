#include "client.h"

int judge(char *buf,ptrain_t train)
{   
    train_t t;
    bzero(&t,sizeof(train_t));
    if(strcmp(buf,"ls")==0){
        t.type=2;
        strcpy(t.buf,"ls");
        t.dataLen=10;//4+4+2
    }
    *train=t;
    return 0;
}

