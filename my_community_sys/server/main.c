
/*
2.编写一个服务器 ,作为小区信息发布系统的服务端，功能如下

1）制定并统一小区通信的加密协议供双方使用

2）客户端链接到服务器后，服务器实时更新当前客户端在线人数！

3）发布公告信息到每一个客户端

4）响应客户端的各类请求，如响应客户端的发送请求，发送文件的请求

*/



#include "my_community_sys.h"

#define SIZE    100
#define INFO_weather "/home/gec/InstantMsg_weather.txt"

#define INFO_news "/home/gec/InstantMsg_news.txt"

#define INFO_dir "/home/gec/client_info/"

#define MAIN

//用于服务器接收客户端文件
 struct info *INFO;//客服段文件暂存区指针


struct usr *head;//好友列表的头指针
int num=0;//存储当前客户端在线人数
int num_info=0;
struct weather w;//存储当前最新的天气信息
char news[4096];//存储日志API数据




//监听多个客户端socket，并响应客户端的各类请求
void *listen_client(void *arg)
{

    int sockfd = *((int *)arg); //最大的描述符
    int max=sockfd;

    //1.把文件描述符集合清空   （先把监狱处理干净）
        fd_set set;


    while(1)
    {
        FD_ZERO(&set);//清空文件描述集合

        //2.把阻塞的文件描述符添加到集合中
        FD_SET(sockfd,&set);
        FD_SET(0,&set);//标准输入

        for(int i=0;i<num;i++)
        {
            FD_SET(head[i].socket,&set);
            if(i == num-1)
            {
                max=head[i].socket;
            }
        }




    //3.监听集合        （看看谁不听话）
       int ret = select(max+1,&set,NULL,NULL,NULL);//设置监听的时长
        if(ret < 0)
        {
            perror("");
            return 0;

        }


        //服务器socket活跃的标准是客户端是否发送了链接请求，这里的socket仅用于判断客户端是否链接。
        if(FD_ISSET(sockfd,&set))  //判断是否  服务器socket  活跃
        {


            //用于保存对方的地址信息
            struct sockaddr_in  clien_addr;
            socklen_t addr_len = sizeof(clien_addr);

            //4.接收客户端的链接请求
            int  newfd=accept(sockfd,(struct sockaddr *)&clien_addr,&addr_len);
            if( newfd < 0)
            {
                perror("");
                return  0;
            }
            else
            {
                printf("链接成功！\n");
                int port = ntohs(clien_addr.sin_port);//本地序

                //存放对方IP地址信息

                head[num].addr.sin_addr.s_addr=clien_addr.sin_addr.s_addr;
                head[num].socket = newfd;
                head[num].addr.sin_port= port;
                num++;
                write(newfd,"loin\r\n\r\n",strlen("loin\r\n\r\n"));//发送loin给客户端，让客户端把用户信息传过来！

            }
        }

        //newsocket用于服务器与客户端的通信，活跃的标准是客户端发来通信数据

        //遍历当前在线好友列表，判断是否有信息接收！接收所有好友的信息！
        for(int i=0;i<num;i++)
        {
            if(head[i].socket != 0)
            {
                if(FD_ISSET(head[i].socket,&set))   //判断是否新的描述符活跃
                {
                    char  buf[5000]={0};
                    int ret=read(head[i].socket,buf,5000);

                    //这里以新的描述符活跃但读得数据小于0来判定用户下线！
                    if(ret <= 0)
                    {
                        printf("下线通知：\n");
                        printf("用户: %s已下线\n",head[i].name);
                        //处理下线用户
                       del_usr(&head[i],i);
                       continue;
                    }
                    char msg[2048]={0};
                    //制定一个小区加密协议来管理用户信息，响应用户请求

                   //接收客户端的用户信息
                    if(strstr(buf,"usr_msg") != NULL)
                    {
                        sscanf(buf,"usr_msg:\r\n[name:%[^]]",head[i].name);//这里以用户的姓名为例
                        char *p=strstr(buf,"house");
                        sscanf(p,"house:%[^]]",head[i].house);
                        show_friend(head);
                        bzero(buf,sizeof(buf));
                        continue;
                    }


                    //这里设置只有获取到了用户的信息才接收用户的信息！
                    if(strlen(head[i].name) != 0 )
                    {


                        //响应客户端发送信息的请求！
                        if(strstr(buf,"write2usr") != NULL)
                        {
                            char write_buf[500]={0};
                            char read_buf[500]={0};
                            char information[4096]={0};
                            char info[4096]={0};
                            //write2usr:\r\nwrite=[%s]info=[%s]read=[%s]\r\n\r\n
                            sscanf(buf,"write2usr:\r\nwrite=[%[^]]",write_buf);

                            char *p1=strstr(buf,"info=");
                            sscanf(p1,"info=[%[^]]",info);

                            char *p2=strstr(buf,"read=");
                            sscanf(p2,"read=[%[^]]",read_buf);


                            printf("write:%s,info:%s,read:%s\n",write_buf,info,read_buf);
                             struct usr read_usr=Findusr(head,read_buf,0);
                            sprintf(information,"read_msg:\r\n[%s]: %s\r\n\r\n",write_buf,info);
                            if(strlen(read_usr.name) != 0)
                            {
                                write(read_usr.socket,information,sizeof(information));
                            }

                            else
                            {
                                write(head[i].socket,"usr not exist or has outline\r\n\r\n",strlen("usr not exist or has outline\r\n\r\n"));
                            }

                        }

                        //接收客户端收到公告的确认信息,表示客户端看到了公告信息
                        if(strstr(buf,"usrRECVnotice") != NULL)
                        {
                           struct usr write_usr=Findusr(head,NULL,head[i].socket);//找到发送者
                            printf("usr：[%s]已收到公告信息!\n",write_usr.name);

                        }



                        //响应客户端获取实时信息的要求
                        if(strstr(buf,"GetInstantMsg") != NULL)
                        {
                            //printf("GetInstantMsg\n");


                            //获取天气
                             struct weather w=Json2weather();//获取天气
                             char buf_data[4096]={0};
                             char msg_head[100]={"RecvInstantMsg\r\n"};
                             char msg_tail[10]={"\r\n\r\n"};
                             memcpy(buf_data,msg_head,strlen("RecvInstantMsg\r\n"));
                             memcpy(buf_data+strlen("RecvInstantMsg\r\n"),(char *)&w,sizeof(w));
                             memcpy(buf_data+strlen("RecvInstantMsg\r\n")+sizeof(w),msg_tail,strlen("\r\n\r\n"));

                               write(head[i].socket,buf_data,4096);

                            /*
                             *
                             * 测试发现，如果用write分条发送，客户端接收消息可能会被截断！！！！！所以必须按照消息头 + 数据  + \r\n\rn合并发送！！！
                             //先发送消息头
                            write(head[i].socket,"RecvInstantMsg\r\n",strlen("RecvInstantMsg\r\n"));

                            int ret=write(head[i].socket,buf_data,sizeof(w));//表示结束！
                            if(ret <= 0)
                            {
                                perror("发送天气信息失败！");
                            }

                            printf("LINE:%d\n",__LINE__);
                            write(head[i].socket,"\r\n\r\n",4);//表示结束！
                             */


                        }
                        if(strstr(buf,"GetNewsMsg") != NULL)
                        {

                            if(strlen(news) != 0)
                            {
                                char buf_data[2048]={0};
                                sprintf(buf_data,"RecvNewsMsg\r\n%s\r\n\r\n",news);
                                write(head[i].socket,buf_data,2048);

                            }

                        }

                        //响应客户端发送文件的请求,获取文件名和文件大小！
                        if(strstr(buf,"transfer_info_ready") != NULL)
                        {
                            printf("transfer_info_ready!!!!\n");
                            char non1[50];
                            char non2[50];
                            char write_buf[500]={0};
                            char read_buf[500]={0};
                            char info_name[4096]={0};
                            int info_size=0;
                            char information[4096]={0};
                            sscanf(buf,"%s %s %d %s %s %s",non1,write_buf,&info_size,info_name,read_buf,non2);

                            printf("write_buf:%s,info_size:%d,info_name:%s,read_buf:%s\n",write_buf,info_size,info_name,read_buf);



                            strcpy(INFO[num_info].info_name,info_name);
                            strcpy(INFO[num_info].read_buf,read_buf);
                            strcpy(INFO[num_info].write_buf,write_buf);
                            INFO[num_info].info_size=info_size;
                            INFO[num_info].socket=head[i].socket;
                            char info_data[1024]={0};
                            sprintf(info_data,"%s%s_%s",INFO_dir,write_buf,info_name);//服务器中的文件名加上发送者的名字，以免搞混
                            int fd=open(info_data,O_RDWR|O_CREAT|O_TRUNC,0777);
                            if(fd <0)
                            {
                                perror("");
                            }
                            INFO[num_info].fd=fd;
                            num_info++;


                            struct usr write_usr=Findusr(head,write_buf,0);//找到发送者
							
                            //printf("write_info_name:%s\n",write_usr.name);

                            //向客户端写一个收到请求的响应文，让客户端开始发送文件
                            sprintf(information,"please2SendInfo\r\n\r\n");
                            if(write_usr.socket != 0)
                            {

                                int ret=write(head[i].socket,information,strlen(information));
                                if(ret <=0)
                                {
                                    perror("");
                                }

                            }
                        }



                             //接收发送者的文件，存储到服务器中！
                        if(strstr(buf,"Info_Send:\r\n\r\n") != NULL)
                        {              
                            for(int i;i<num_info;i++)
                            {

                                if(INFO[i].fd  > 0)//如果文件暂存区有该用户的文件就准备传输！

                                {
                                    int down_size=0;
                                    char buf_down[5000]={0};

                                    while(1)
                                    {
                                        bzero(buf_down,sizeof(buf_down));

                                         int ret2=read(INFO[i].socket,buf_down,5000);//没有数据会阻塞！
                                         if(ret2 <= 0)
                                         {
                                             perror("read:");
                                         }



                                         //这里的判断是多余的，无需判断，原因是文件头后面的数据必然是原始数据！
                                         //if(strstr(buf_down,"Info_Send:\r\n\r\n") != NULL)
                                        // {
                                              int ret1 = write(INFO[i].fd,buf_down+strlen("Info_Send:\r\n\r\n"),ret2);
                                              if(ret1 <=0)
                                              {
                                                  perror("write");
                                              }
                                             down_size+=ret1;
                                             //printf("down_size:%d\n",down_size);
                                             printf("下载进度：%%%d\n",(int)((float)down_size/INFO[i].info_size)*100);
                                        // }

                                         if(down_size ==INFO[i].info_size)
                                         {
                                             break;
                                         }

                                    }
                                    printf("已收到发送者[%s]的文件！\n",INFO[i].write_buf);

                                    close(INFO[i].fd);
                                     INFO[i].fd=0;
                                    INFO[i].flag=1;//表明文件发送完毕！

                                }
                            }
                        }


                        if(strstr(buf,"reject the info!\r\n\r\n") != NULL)
                        {

                            for(int i=0;i<num_info;i++)
                            {
                                if(head[i].socket == INFO[i].socket)//找到接收文件的客户端
                                {
                                   INFO[i].flag_Recv = -1;//表示拒收！

                                }

                            }


                        }


                        if(strstr(buf,"accept the info!\r\n\r\n") != NULL)
                        {

                            for(int i=0;i<num_info;i++)
                            {
                                if(head[i].socket == INFO[i].socket)//找到接收文件的客户端
                                {
                                   INFO[i].flag_Recv = 1;//表示接收！

                                }

                            }


                        }

                        if(strstr(buf,"info accept success!\r\n\r\n") != NULL)
                        {

                            for(int i=0;i<num_info;i++)
                            {
                                if(head[i].socket == INFO[i].socket)//找到接收文件的客户端
                                {
                                   INFO[i].flag_Recv = 3;//表示接收完毕！

                                }

                            }


                        }



                    }
                    bzero(buf,sizeof(buf));
                    bzero(msg,sizeof(msg));
                }
            }

       }


    }



}



//处理文件暂存区的发送任务
void *file_Send(void *arg)
{
    while(1)
    {
        for(int i=0;i<num_info;i++)
        {

            if(INFO[i].flag ==1)//表示有任务
            {
                char buf_data[100]={0};
                sprintf(buf_data,"Send2usr: %s %s %d \r\n\r\n",INFO[i].write_buf,INFO[i].info_name,INFO[i].info_size);
                write(INFO[i].socket,buf_data,strlen(buf_data));
                printf("%s\n",buf_data);

                while(INFO[i].flag_Recv ==0);//等待客户端响应  初始状态为0


                if(INFO[i].flag_Recv ==-1)//拒收
                {
                    INFO[i].flag=2;//表示处理完毕！
                    continue;

                }

                if(INFO[i].flag_Recv ==1)//客户端接收
                {
                    char info_data[100]={0};
                    char buf_down[4096]={0};
                    int down_size=0;
                    sprintf(info_data,"%s%s_%s",INFO_dir,INFO[i].write_buf,INFO[i].info_name);//服务器中的文件名加上发送者的名字，以免搞混
                    int fd= open(info_data,O_RDWR,0777);
                    if(fd <0)
                    {
                        perror("");
                    }
                    while(1)
                    {
                        bzero(buf_down,sizeof(buf_down));
                        int ret1=read(fd,buf_down,4096);
                        if(ret1 <=0)
                        {
                            break;
                        }
                        down_size+=ret1;
                        printf("down_size:%d\n",down_size);
                        int ret2=write(INFO[i].socket,buf_down,ret1);
                        if(ret2 <=0)
                        {
                            perror("write");
                        }

                    }
                    printf("发送完毕！\n");
                    while(INFO[i].flag_Recv ==1);//等待客户端接收完毕！
					
                    INFO[i].flag=2;//表示处理完毕！
					//bzero(INFO[i],sizeof(INFO[i]0));   ??  ->   此时INFO[i].flag=0 表示结束


                }



            }

        }

    }


}

void show_friend(struct usr * head)
{
       printf("--------------------------------------------------------\n");
    printf("当前在线人数：%d\n",num);
    for(int i=0;i<num;i++)
    {
        char  *ip  = inet_ntoa(head[i].addr.sin_addr);
        printf("%d. name: %s house:%s ip:%s  port:%d\n",i+1,head[i].name,head[i].house,ip,head[i].addr.sin_port);
    }
        printf("--------------------------------------------------------\n");
    return;

}



//结构体为空表示好友不存或已经下线,find_name表示按名字查找，find_socket表示套接字文件描述符查找  没有填写NULL，或者0
 struct usr Findusr(struct usr * head,char *find_name,int find_socket)
{
    int flag=0;
    struct usr client;
    bzero(&client,sizeof(client));
    //在当前在线好友列表中找到该好友！
    if(find_name != NULL)
    {
        for(int i=0;i<num;i++)
        {
            if(strstr(head[i].name,find_name) != NULL)
            {

                flag=1;
                 client=head[i];
                 return client;
            }

        }
        if(flag ==0)
        {
            printf("该好友不存在或已经下线！\n");
            return client;
        }

    }

    else if(find_socket != 0)
    {
        for(int i=0;i<num;i++)
        {
            if(find_socket == head[i].socket)
            {

                flag=1;
                client=head[i];
                 return client;
            }

        }
        if(flag ==0)
        {
            printf("该好友不存在或已经下线！\n");
            return client;
        }

    }

    else
    {
        printf("该好友不存在或已经下线！\n");
        return client;
    }
    return client;

}


//发布公告
int SendNotice(struct usr * head)
{
    printf("请输入公告信息\n");
    char notice[4096]={0};
    char buf_data[4096]={0};
    scanf("%s",buf_data);
    sprintf(notice,"SendNotice\r\n%s\r\n\r\n",buf_data);
    for(int i=0;i<num;i++)
    {
        write(head[i].socket,notice,sizeof(notice));
    }
    printf("小区公告信息发送完毕！\n");
    return 0;

}



int del_usr(struct usr * p,int p_num)
{
    bzero(&p[p_num],sizeof(struct usr));
    for(int i=p_num;i<num;i++)
    {
        if(i ==num-1)
        {
            bzero(&p[i],sizeof(struct usr));
        }
        p[i]=p[i+1];

    }
    num--;
    return 0;


}
#ifdef MAIN

int main()
{
      //创建一个好友列表
     struct usr *friend=malloc(sizeof(struct usr) * SIZE);
     head=friend;
     bzero(friend,sizeof(struct usr) * SIZE);

     //创建一个文件结构体，负责存储用户发送过的文件信息
     INFO=malloc(sizeof(struct info) * SIZE);

     //创建一个用户的文件目录来存储客户文件
     mkdir(INFO_dir,0777);

    //创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==0)
    {
        perror("");
        return -1;
    }

    //SO_REUSEADDR
    //解决端口复用
    int on=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    on=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,&on,sizeof(on));

    //绑定自己的IP信息
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(10086);//端口号
    addr.sin_addr.s_addr=INADDR_ANY;//当前192.168.22.16
    int ret=bind(sockfd,(struct sockaddr *)&addr,sizeof(addr));
    if(ret<0)
    {
        perror("bind");
        return -1;

    }

    //监听
    listen(sockfd,6);


    //创建一个子线程 用于服务器监听多个客户端socket
    pthread_t  pid;
    pthread_create(&pid,NULL,listen_client,&sockfd);


    //创建一个子线程 用于服务器监听多个客户端socket
    pthread_t  pid_Send;
    pthread_create(&pid_Send,NULL,file_Send,NULL);




      char link1[4096]={"https://tianqiapi.com/api?version=v6&appid=39134871&appsecret=ii67BlwB&city=广州"};//存放HTTP链接,默认的链接是获取实时天气相关信息
     //GetMsg_from_http(link1,1);//获取天气等实时信息


      //获取舔狗日志
      char link2[4096]={"https://v1.alapi.cn/api/dog"};//舔狗日志API
      //GetMsg_from_http(link2,2);
      char * p=Json2news();//该函数内部会创建一个堆空间，使用后记得释放！
      memcpy(news,p,strlen(p));
      printf("news:%s\n",news);
      free(p);



    while(1)
    {
        //system("clear");
        printf("--------------------------------------------------------\n");
        printf("请选择功能：  1.打印当前在线好友列表   2.发布小区公告   3.获取实时信息    4.退出\n");
        printf("--------------------------------------------------------\n");
        int a=0;

        scanf("%d",&a);
        switch(a)
        {
            case 1:
                    system("clear");
                    show_friend(head);
                    break;


            case 2:
                    system("clear");
                    SendNotice(head);
                    break;

            case 3:
                    system("clear");
                    GetMsg_from_http(link1,1);//获取天气等实时信息
                    GetMsg_from_http(link2,2);//获取新闻等实时信息
                    sleep(1);
                    break;

            case 4:
                    return 0;

            default:
                    break;

        }

    }
    return 0;

}

#endif
