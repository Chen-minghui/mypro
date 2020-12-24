#ifndef MY_COMMUNITY_SYS_H
#define MY_COMMUNITY_SYS_H

#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include "cjson.h"



//用户信息结构体
struct usr
{
    struct sockaddr_in addr;
    char name[1024];
    char house[1024];
    int socket;
};


struct weather
{
char date[100];
char week[100];
char city[100];
char wea[100];
char  tem[50];
char  L_tem[50];
char  H_tem[50];
char win[100];
char win_speed[50];
char humidity[100];
char  air[50];
char air_L[50];
char air_tips[1000];

}weather;



//用户文件转发任务的暂存区
struct info
{
    int socket;//发送者的socket
    char write_buf[500];//发送者姓名
    char read_buf[500];//接收者姓名
    char info_name[1024];//文件名
    int info_size;//文件大小
    int fd;//存放发送者的文件描述符
    int flag;//1为等待服务器发送，0为无任务，2为服务器处理完毕（包括服务器发送完毕或客户端拒收）
    int flag_Recv;//标志接收者是否接收！  1  接收  -1  拒收     3接收完毕！  0  初始空状态
}info;


int del_usr(struct usr * p,int p_num);
void *listen_client(void *arg);
void show_friend(struct usr * head);
int Send2usr(struct usr * head);
int SendNotice(struct usr * head);
struct usr Findusr(struct usr * head,char *find_name,int find_socket);


int get_ip_from_host(char *ipbuf, const char *host, int maxlen);
char *strrev(char *str);
int readline(char *buf,int max_size,int fd);
int GetMsg_from_http(char *link,int flag);
struct weather Json2weather(void);
char * Json2news(void);




#endif // MY_COMMUNITY_SYS_H
