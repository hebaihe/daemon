#include "client.h"
#include "md5.h"
#define _XOPEN_SOURCE

#define READ_DATA_SIZE 1024
#define MD5_SIZE 16
#define MD5_STR_LEN (MD5_SIZE * 2)

int Compute_file_md5(const char *file_path, char *value);
int ret;
int check_dir(int socketFd, char *buf) //检查当前目录是否有待上传的file,有则直接trans_file
{
	train_t taskTrain;
	printf("buf=%s\n", buf);
	char *file = (char *)malloc(128);
	memset(file, 0, 128);
	sscanf(buf, "%*s%s", file); //dir取空格后的字符串
	printf("file=%s\n", file);
	DIR *pdir = opendir("./"); //打开当前目录
	if (pdir == NULL)
	{
		printf("upload fail\n");
		return -1;
	}
	else
	{
		struct dirent *mydir;					//创建目录结构体
		while ((mydir = readdir(pdir)) != NULL) //每次readdir后指针会自动偏移到下一个目录
		{
			if (strncmp(mydir->d_name, ".", 1) == 0 || strncmp(mydir->d_name, "..", 2) == 0)
			{
				continue;
			}
			if (strcmp(mydir->d_name, file) == 0)
			{
				printf("reading the file...\n");
				const char *file_path = file;
				char md5_str[MD5_STR_LEN + 1];
				ret = Compute_file_md5(file_path, md5_str);
				if (0 == ret)
				{
					printf("[file - %s] md5 value:%s\n", file_path, md5_str);
					bzero(buf, 1000);
					sprintf(buf, "%s %s %s", "upload", file, md5_str); //buf=upload file md5sum，不重新组合会有\n+md5
					printf("buf:%s\n", buf);
					printf("uploading...\n");
					taskTrain.dataLen = strlen(buf);
					//buf是我输入的任务，需要把它组合成upload md5 :bzero(buf,1000); sprintf(buf,"%s %s","upload",md5);
					strcpy(taskTrain.buf, buf);
					send(socketFd, &taskTrain, 4 + taskTrain.dataLen, 0);
					return 0;
				}
			}
		}
	}
	printf("upload fail\n");
	return -1;
}

int check_salt(int socketFd, char *password) //密码验证专用，成功输入用户名，得到对方发回的盐值，然后crypt(password,salt)生成密文发给server
{
	int len;
	char salt[32] = {0};
	recvCycle(socketFd, &len, 4);
	recvCycle(socketFd, salt, len);
	char *pwd = crypt(password, salt);
	len = strlen(pwd); //此处buf不含\n不需要len-1
	printf("salt=%s\n", salt);
	printf("len=%d\n", len);
	printf("pwd=%s\n", pwd);
	send(socketFd, &len, 4, 0);
	send(socketFd, pwd, len, 0);
	return 0;
}
int Compute_file_md5(const char *file_path, char *md5_str)
{
	int i;
	int fd;
	int ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX md5;

	fd = open(file_path, O_RDONLY);
	if (-1 == fd)
	{
		perror("open");
		return -1;
	}

	// init md5
	MD5Init(&md5);

	while (1)
	{
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret)
		{
			perror("read");
			return -1;
		}

		MD5Update(&md5, data, ret);

		if (0 == ret || ret < READ_DATA_SIZE)
		{
			break;
		}
	}

	close(fd);

	MD5Final(&md5, md5_value);

	for (i = 0; i < MD5_SIZE; i++)
	{
		snprintf(md5_str + i * 2, 2 + 1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end

	return 0;
}