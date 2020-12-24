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
#include "cjson.h"


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


#define INFO_weather "/home/gec/InstantMsg_weather.txt"

#define INFO_news "/home/gec/InstantMsg_news.txt"


/*
作用：将网络地址转化为IP
参数：ipbuf是输出缓冲区, host是要转化的域名, maxlen是缓冲区大小
返回值：返回-1是失败，0是成功
*/
int get_ip_from_host(char *ipbuf, const char *host, int maxlen)
{
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    if (inet_aton(host, &sa.sin_addr) == 0) //由本地序转化为网络序投向网络使用
    {
        struct hostent *he;
        he = gethostbyname(host);//由域名获取IP地址,#include <netdb.h>
        if (he == NULL)
            return -1;
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    strncpy(ipbuf, inet_ntoa(sa.sin_addr), maxlen);//存放于ipbuf中
    return 0;
}

char *strrev(char *str){
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}


//制作一个读取某行字符串的函数，用于读取系统IO的文件
//参数：buf是输出缓冲区, max_size是缓冲区大小,fd为文件描述符
//返回值：返回-1是失败，0是成功
int readline(char *buf,int max_size,int fd)
{
    int sum=0;
    while(1)
    {
        int ret=read(fd,buf+sum,1);

        if(ret ==-1)
        {
            perror("");
            return -1;
        }
        if(ret ==0)
        {
          break;
        }
        sum +=ret;
        if(max_size ==sum)//满了就退出！
        {
            break;
        }

        if(buf[sum-1] =='\n')//遇到\n表示读到了一行，退出！
        {
            break;
        }
    }
    return 0;

}




//flag->1获取天气   flag->2获取新闻....
int GetMsg_from_http(char *link,int flag)
{



    char web1[4096]={0};//strtok会改变原字符串web,所以再创建一个
    strcpy(web1,link);



    int  fd;

    if(flag ==1)
    {
        //新建文件
        fd=  open(INFO_weather,O_RDWR|O_CREAT|O_TRUNC,0777);
            if(fd < 0)
            {
                perror("");
                return 0;

            }
    }




    if(flag ==2)
    {
        //新建文件
            fd=  open(INFO_news,O_RDWR|O_CREAT|O_TRUNC,0777);
            if(fd < 0)
            {
                perror("");
                return 0;

            }
    }


        //重点！！定制HTTP 请求协议
        //   https://file.alapi.cn/image/comic/214256-15045325768a2c.jpg


        //存储host
       char host[4096]={0};
       if(strstr(web1,"https://") != NULL)
       {
           sscanf(web1,"https://%s",host);
       }
       if(strstr(web1,"http://") != NULL)
       {
           sscanf(web1,"http://%s",host);
       }
       strtok(host,"/");

       //存储URL
       char URL[4096]={0};
       int num=0;
       int j=0;
       for(int i=0;i<(int)strlen(web1);i++)
       {
           if(web1[i] =='/' )
           {
               num++;
           }

           if(num >=3)
           {
               URL[j++]=web1[i];
           }

       }

       char  http[4096]={0};
      sprintf(http,"GET %s HTTP/1.1\r\nHost:%s\r\n\r\n",URL,host);
      printf("%s\n",http);


        //1.新建TCP 通信对象
        int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

        //2.链接服务器
         //设置服务器的IP地址信息

        //域名解析，返回可以直接使用的整型IP地址

       char ipbuf[1024]={0};
       get_ip_from_host(ipbuf,host, sizeof(ipbuf));//结果存储在ipbuf中，返回值判断是否成功！

        struct sockaddr_in  addr;
        addr.sin_family   = AF_INET; //IPV4 协议
        addr.sin_port     = htons(80); //端口 80  ,所有的HTTP 服务器端口都是  80
        addr.sin_addr.s_addr = inet_addr(ipbuf); //服务器的IP 地址信息
       int ret=connect(tcp_socket,(struct sockaddr *)&addr,sizeof(addr));
           if(ret < 0)
           {
               perror("");
               return 0;
           }
           else
           {
               printf("链接网络服务器成功\n");
           }




        //发送数据给服务器
       write(tcp_socket,http,strlen(http));
       printf("%s\n",http);

        int file_size=0;
        char head_buf[1024]={0};


       sleep(1);


        while(1)
        {

          readline(head_buf,1024,tcp_socket);
          printf("%s\n",head_buf);
          sscanf(head_buf,"Content-Length: %d",&file_size);

          if(head_buf[0]=='\r'&&head_buf[1]=='\n'&& strlen(head_buf)==2)//判断http回文的数据是否到头！，即只有/r/n
          {
              break;
          }
          bzero(head_buf,1024);

        }

        int down_size=0;//目前下载的字节数


        if(file_size != 0)
        {
            while(1)
            {
                //获取服务器的信息
                char  buf[4096]={0};
                int size=read(tcp_socket,buf,4096);
                //printf("buf:%s\n",buf);
                    write(fd,buf,size);
                    down_size+=size;
                    if(down_size == file_size )
                    {
                        printf("下载完毕!\n");
                        break;
                    }


            }

        }


        //测试发现https://v1.alapi.cn/api/tianqi/now没有回传的文件大小，所以遇到author表示结束退出

        else
        {      
            int size=0;

            do
            {
                //获取服务器的信息
                char  buf[4096]={0};
                int size=read(tcp_socket,buf,4096);
                //printf("buf:%s\n",buf);
                    write(fd,buf,size);
                    down_size+=size;
                    if(strstr(buf,"author") != NULL )
                    {
                        printf("下载完毕! \n");
                        break;
                    }

            }while(size > 0);
        }



    close(fd);
    close(tcp_socket);
    return 0;


}


struct weather Json2weather(void)
{
    struct weather w;
    bzero(&w,sizeof(w));
    char buf[4096]={0};
    //打开JSON数据文件
        int fd = open(INFO_weather,O_RDWR,0777);
        if(fd < 0)
        {
            perror("open fail\n");
            return w;
        }

        //读取文件中的数据
      int ret=read(fd,buf,4096);
      if(ret <= 0)
      {
          perror("");
      }


        //关闭文件
        close(fd);

        char json_data[5000]={0};
        sscanf(buf,"%*[^{]%[^}]}",json_data);
        json_data[strlen(json_data)]='}';
        json_data[strlen(json_data)]='}';
        json_data[strlen(json_data)+1]='\0';


       // printf("json_data=%s\n",json_data);
        //把该字符串数据转换成JSON对象
        cJSON *root=cJSON_Parse(json_data);
        if(root == NULL)
        {
            printf("parse error\n");
            return w;
        }


         cJSON *value;


            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"date");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *date = cJSON_Print(value);
            sscanf(date,"\"%[^\"]",w.date);
           // printf("w.date=%s\n",w.date);


            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"week");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *week = cJSON_Print(value);
            sscanf(week,"\"%[^\"]",w.week);
            //printf("w.date=%s\n",w.week);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"city");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *city = cJSON_Print(value);
            sscanf(city,"\"%[^\"]",w.city);
            //printf("w.date=%s\n",w.city);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"wea");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *wea = cJSON_Print(value);
            sscanf(wea,"\"%[^\"]",w.wea);
            //printf("w.date=%s\n",w.wea);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"tem");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *tem = cJSON_Print(value);
            sscanf(tem,"\"%[^\"]",w.tem);
            //printf("w.date=%s\n",w.tem);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"tem1");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *tem1 = cJSON_Print(value);
            sscanf(tem1,"\"%[^\"]",w.H_tem);
            //printf("w.date=%s\n",w.H_tem);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"tem2");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *tem2= cJSON_Print(value);
            sscanf(tem2,"\"%[^\"]",w.L_tem);
            //printf("w.date=%s\n",w.L_tem);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"win");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *win = cJSON_Print(value);
            sscanf(win,"\"%[^\"]",w.win);
            //printf("w.date=%s\n",w.win);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"win_speed");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *win_speed = cJSON_Print(value);
            sscanf(win_speed,"\"%[^\"]",w.win_speed);
           // printf("w.date=%s\n",w.win_speed);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"humidity");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *humidity = cJSON_Print(value);
            sscanf(humidity ,"\"%[^\"]",w.humidity);
            //printf("w.date=%s\n",w.humidity);

            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"air");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *air = cJSON_Print(value);
            sscanf(air,"\"%[^\"]",w.air);
            //printf("w.date=%s\n",w.air);




            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"air_level");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *air_level = cJSON_Print(value);
            sscanf(air_level,"\"%[^\"]",w.air_L);
           // printf("w.date=%s\n",w.air_L);


            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"air_tips");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return w;
            }
            //把数据转成 字符串输出
            char  *air_tips = cJSON_Print(value);
            sscanf(air_tips,"\"%[^\"]",w.air_tips);
            //printf("w.date=%s\n",w.air_tips);


        return w;
}


char * Json2news(void)
{

    char buf[4096]={0};
    //打开JSON数据文件
        int fd = open(INFO_news,O_RDWR,0777);
        if(fd < 0)
        {
            perror("open fail\n");
            return NULL;
        }

        //读取文件中的数据
      int ret=read(fd,buf,4096);
      if(ret <= 0)
      {
          perror("");
      }


        //关闭文件
        close(fd);

        char json_data[5000]={0};
        sscanf(buf,"%*[^{]%[^}]}",json_data);
        json_data[strlen(json_data)]='}';
        json_data[strlen(json_data)]='}';
        json_data[strlen(json_data)]='\0';


        //把该字符串数据转换成JSON对象
        cJSON *root=cJSON_Parse(json_data);
        if(root == NULL)
        {
            printf("parse error\n");
            return NULL;
        }


             cJSON *value;
             cJSON *value_content;
            //根据key值去获取对应的value
            value = cJSON_GetObjectItem(root,"data");
            if(value == NULL)
            {
              printf("GetObjec error\n");
              return NULL;
            }

            value_content= cJSON_GetObjectItem(value,"content");
            //把数据转成 字符串输出
            char  *content = cJSON_Print(value_content);

            char *news=malloc(strlen(content)-2);//申请一段空间存放news
            memcpy(news,content+1,strlen(content)-2);//拷贝到news中去

        return news;





}

