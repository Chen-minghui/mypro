#include <stdio.h>
#include <netinet/ip.h> /* superset of previous */
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h> 
#include <sys/mman.h>
#include <time.h>
#include <pthread.h>
#include "font.h"


	char usr_addr_buf[100]={0};
	char usr_name[50]={0};
    char personal_information[500]={0};
	struct weather w;//存储当前最新的天气信息
	char recv_msg[1000]={0};//聊天内容
	char Notice_new[100]={0};//最新公告
	char Notice_all[1024]={0};//所有公告
	int tcp_socket;
	int n=0;//判断是否为第一次收公告或刷新了公告
	int i=0;//判断是否为第一次收信息或刷新了信息
	int num=0;//判断所在界面
	unsigned int *lcd_fd;//初始化LCD的内存映射
	char city[100]={0};
	char wea[100]={0};
	char tem[100]={0};
	char H_tem[100]={0};
	char L_tem[100]={0};
	char win[100]={0};
	char win_speed[100]={0};
	char humidity[100]={0};
	char air[100]={0};
	char air_L[100]={0};
	char rizhi[1000]={0};
	
	//这里实际上的客户端应该在开发板的家目录
#define INFO_weather "/home/gec/InstantMsg_weather.txt"
#define  TS_MODE  1
#define INFO_news "/home/gec/InstantMsg_news.txt"
	
	
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
int readline(char *buf,int max_size,int fd);
struct location read_xy();
int show_bmp_quickly(int x, int y, int bmp_w, int bmp_h,char *bmp_name); //快速显示图片
void input_word2(char *name,int word_size,int xh,int yz,int k_wide,int k_hight);//lcd显示文字
int lcd_init();



char  file_num=0;

struct location 
{
	int x1;
	int y1;
	int x2;
	int y2;
	int type;
	int left_move;
	int right_move;
};

//struct location read_xy();
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





//接受服务器数据的子线程！
void * Recv(void *arg)
{
	fd_set set;
    int max_fd=1;
	
	//读取服务器发送过来的数据 
	while(1)
	{

        FD_ZERO(&set);
        FD_SET(0,&set);
        if(max_fd < 0)
        {
            max_fd=0;
        }
		

		
        FD_SET(tcp_socket,&set);
        if(max_fd < tcp_socket)
        {
            max_fd = tcp_socket;
        }
		//select多路复用
        int ret = select(max_fd + 1, &set, NULL, NULL, NULL);
		if(ret < 0)   
		{   
		perror("select failure\n");   
		continue ;
		}   
 
		if(FD_ISSET(tcp_socket,&set))
		{

			char read_buf[4096]={0}; 
			read(tcp_socket,read_buf,4096);
			if(strstr(read_buf,"read_msg:\r\n")!=NULL)//接收其他用户消息
			{
				if(i==1)
				{
				strcat(recv_msg,read_buf);
				}
				if(i==0)
				{
				strcpy(recv_msg,read_buf);
				i++;
				}

			}
			
			if(strstr(read_buf,"RecvInstantMsg\r\n")!=NULL)//接收天气信息
			{
		//w为天气结构体，全局变量，自动更新最新的天气！
					
					bzero(&w,sizeof(w));
					memcpy((char *)&(w),read_buf+strlen("RecvInstantMsg\r\n"),sizeof(w));
				
					
					
					sprintf(city,"城市：%s",w.city);
					sprintf(wea,"天气：%s",w.wea);
					sprintf(tem,"当前温度：%s",w.tem);
					sprintf(H_tem,"最高温度：%s",w.H_tem);
					sprintf(L_tem,"最低温度：%s",w.L_tem);
					sprintf(win,"风向：%s",w.win);
					sprintf(win_speed,"风速：%s",w.win_speed);
					sprintf(humidity,"湿度：%s",w.humidity);
					sprintf(air,"空气指数：%s",w.air);
					sprintf(air_L,"空气质量：%s",w.air_L);
					
					if(strlen(w.date) ==0)
					{
						char news_buf[1024]={0};
						stpcpy(news_buf,"GetInstantMsg\r\n\r\n");
						write(tcp_socket,news_buf, strlen(news_buf));
						
					}
					
						
					//test:	
					
					if(strlen(w.date) !=0)
					{
					 printf("date:%s    week:%s   city:%s\n",w.date,w.week,w.city);
					 printf("wea:%s    tem:%s   tem1:%s    tem2:%s\n",w.wea,w.tem,w.H_tem,w.L_tem);
					 printf("win:%s    win_speed:%s   humidity:%s\n",w.win,w.win_speed,w.humidity);
					 printf("air:%s    air_level:%s   air_tips:%s\n",w.air,w.air_L,w.air_tips);
						
					}

					 
				 

			}
			
			
			if(strstr(read_buf,"SendNotice")!=NULL)//接收公告
			{	
				char buf[100]={0};
				if(n==1)
				{
				printf("11111\n");
				sscanf(read_buf,"SendNotice%s",Notice_new);
				sprintf(buf,"\n%s",Notice_new);
				strcat(Notice_all,buf);
				
				}
				
				if(n==0)
				{
				sscanf(read_buf,"SendNotice%s",Notice_new);
				strcpy(Notice_all,Notice_new);
				n==1;
				}
				
				
				if(num==0)
				{
				input_word2(Notice_new,100,200,100,400,200);	
				sleep(5);
				show_bmp_quickly(0, 0 ,800, 480,"chat.bmp"); //快速显示图片
				}
				
				if(num==1)
				{			
				input_word2(Notice_new,30,0,0,300,100);
				sleep(5);
				show_bmp_quickly(0, 0 ,800, 480,"chat1.bmp"); //快速显示图片
				}
				
				if(num==2)
				{
					input_word2(Notice_all,100,0,100,700,380);
				}
				
				if(num==3)
				{
					input_word2(Notice_new,100,200,100,400,200);
					sleep(5);
					show_bmp_quickly(0, 0 ,800, 480,"other.bmp"); //快速显示图片
				}
				
				if(num==4)
				{
					input_word2(Notice_new,100,200,100,400,200);
					sleep(5);
					show_bmp_quickly(0, 0 ,800, 480,"weather.bmp"); //快速显示图片
					input_word2(w.date,40,0,100,350,54); //字体大小  框x坐标  y坐标  框宽  高
			
					input_word2(w.week,40,350,100,350,54);
					
					input_word2(city,40,0,154,350,54);
					
					input_word2(wea,40,350,154,350,54);
					
					input_word2(tem,40,0,208,350,54);
					
					input_word2(H_tem,40,350,208,350,54);
					
					input_word2(L_tem,40,0,262,350,54);
					
					input_word2(win,40,350,262,350,54);
					
					input_word2(win_speed,40,0,316,350,54);
					
					input_word2(humidity,40,350,316,350,54);
					
					input_word2(air,40,0,370,350,54);
					
					input_word2(air_L,40,350,370,350,54);
					
					input_word2(w.air_tips,30,0,424,700,54);
				}
				
				if(num==5)
				{
					input_word2(Notice_new,100,200,100,400,200);
					sleep(5);
					show_bmp_quickly(0, 0 ,800, 480,"dog.bmp"); //快速显示图片
					input_word2(rizhi,50,0,100,800,300);
					
				}
			}
			
			
			 if(strstr(read_buf,"loin\r\n\r\n")!=NULL)
			{
					write(tcp_socket,personal_information,sizeof(personal_information));
					printf("已回发个人信息给主机！\n");
			} 
			
			 if(strstr(read_buf,"RecvNewsMsg\r\n")!=NULL) //获取日志
			{
					strcpy(rizhi,read_buf+strlen("RecvNewsMsg\r\n"));
					
					
			}
			
			
			/* if(strstr(read_buf,"ready2RECVinfo\r\n\r\n"))
			{
				file_num++;
				sprintf(new_name,"%s/%s",file_num);
				//strcat(new_name,file_num);
				int fd=open(new_name,O_RDWR|O_CREAT|O_TRUNC,0777);
				if(fd < 0)
				{
					perror("new_name open failed!\n");
					return 0;
				}
				char *p = calloc(1,1024);
				
				while(1)
				{
					
				
					int ret = read(read_buf,p,1024);
					if(ret == 1)//成功读取1024个字节，并写入新文件
					{
						write(fd,p,1024);
						
					}
					
					
				}
				
				close(fd); 
			} */
			
			if(strstr(read_buf,"usr not exist or has outline\r\n\r\n")!=NULL)
			{
				printf("usr not exist or has outline!\n");
			}
		}
			if(FD_ISSET(0,&set)) //标准输入是否存在于ser_fdset集合中（也就是说，检测到输入时，做如下事情）
            {
			printf("11111\n");
            char buf[1024]={0};
			char msg[100]={0};
			char recv[100]={0};
			read(0,buf,1024); 
			sscanf(buf,"%s %s",msg,recv);
			
			if(num==1)//聊天界面
			{
				sprintf(buf,"write2usr:\r\nwrite=[%s]info=[%s]read=[%s]\r\n\r\n",usr_name,msg,recv);
				write(tcp_socket,buf,strlen(buf));
					
            }
		
		
			}
	

	
	
	}
		//关闭通信 
	close(tcp_socket);
	

}

//任务函数 显示接受的信息
void *func2 (void *arg)
{	
	while(1)
	{
	//当在聊天界面时
	
	while(num==1)
	{	

		//把recv_msg中的信息刷到lcd
		input_word2(recv_msg,30,0,100,700,380);
	}

	}
}


int main(int argc, char *argv[])
{
	
    //  ./main  name  192.168.22.xxx  端口
    if(argc !=5)
    {
        printf("./chat_client  name  house  192.168.22.xxx  端口\n");
        return 0;
    }
	
    char new_name[50]={0};
	strcpy(usr_addr_buf,argv[2]);
	strcpy(usr_name,argv[1]);
	sprintf(personal_information,"usr_msg:\r\n[name:%s]\n[house:%s]\r\n\r\n",usr_name,usr_addr_buf);
	
		//1.创建 TCP  通信协议
     tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_socket < 0)
	{
		perror("");
		return 0; 
	}
	else
	{ 


		printf("创建成功\n");
	}
	
	//设置链接的服务器地址信息 
	struct sockaddr_in  addr;  
	addr.sin_family   = AF_INET; //IPV4 协议  
	addr.sin_port     = htons(atoi(argv[4])); //端口
	addr.sin_addr.s_addr = inet_addr(argv[3]); //服务器的IP 地址
	//2.链接服务器 
	int ret=connect(tcp_socket,(struct sockaddr *)&addr,sizeof(addr));
	if(ret < 0)
	{
		perror("");
		return 0;
	}
	else
	{
		printf("链接服务器成功\n");
	}
	
	lcd_init();
	show_bmp_quickly(0,0,800,480,"chat.bmp");

	 //创建线程判断活跃信息收发
     pthread_t  pid;
     pthread_create(&pid,NULL,Recv,NULL);
	 while(1)
	{
	
	  struct location loc = read_xy();
	  if(loc.x1<260 && loc.y1>100)//进入聊天界面
	  {
		  //创建一个线程刷新聊天记录
		  pthread_t  tid2; 	
		  pthread_create(&tid2,NULL,func2,NULL);
		  
		 num=1;//代表在chat界面
		 show_bmp_quickly(0,0,800,480,"chat1.bmp");
		 while(1)
		 {
			 
			struct location loc1 = read_xy();
			if(loc1.x1>700 && loc1.y1>440)
			{
			num=0;
			show_bmp_quickly(0,0,800,480,"chat.bmp");
			break;
			}
			
			if(loc1.x1>700 && loc1.y1<440 &&loc1.y1>390)
			{
			show_bmp_quickly(0,0,800,480,"chat1.bmp");
			bzero(recv_msg,sizeof(recv_msg));
			i--;
			}
		 }
		 
	  }
	  
	  if(loc.x1>300 && loc.x1<510 && loc.y1>100)//进入公告界面
	  {
		  num=2;
		  show_bmp_quickly(0,0,800,480,"Notice.bmp");
		  //把公告数据输出到lcd中
		  input_word2(Notice_all,100,0,100,700,380);
		  
		  while(1)
		 {
			struct location loc2 = read_xy();
			if(loc2.x1>700 && loc2.y1>440)
			{
			show_bmp_quickly(0,0,800,480,"chat.bmp");
			num=0;
			break;
			}
			
			if(loc2.x1>700 && loc2.y1<440 &&loc2.y1>390)
			{
			show_bmp_quickly(0,0,800,480,"Notice.bmp");
			n==0;
			}
		 }
		  
		  
		  
	  }
	  
	  if(loc.x1>560 && loc.y1>100)//进入other界面
	  {
		 show_bmp_quickly(0,0,800,480,"other.bmp"); 
		  num=3;//代表在other界面
		  while(1)
		  {
		  struct location loc3 = read_xy();
		  if(loc3.x1<400 && loc3.y1>100)
		  {
			num=4;//代表在weather界面
			show_bmp_quickly(0,0,800,480,"weather.bmp"); 
			write(tcp_socket,"GetInstantMsg\r\n\r\n",strlen("GetInstantMsg\r\n\r\n"));
			
		    input_word2(w.date,40,0,100,350,54); //字体大小  框x坐标  y坐标  框宽  高
			
			input_word2(w.week,40,350,100,350,54);
			
			input_word2(city,40,0,154,350,54);
			
			input_word2(wea,40,350,154,350,54);
			
			input_word2(tem,40,0,208,350,54);
			
			input_word2(H_tem,40,350,208,350,54);
			
			input_word2(L_tem,40,0,262,350,54);
			
			input_word2(win,40,350,262,350,54);
			
			input_word2(win_speed,40,0,316,350,54);
			
			input_word2(humidity,40,350,316,350,54);
			
			input_word2(air,40,0,370,350,54);
			
			input_word2(air_L,40,350,370,350,54);
			
			input_word2(w.air_tips,30,0,424,700,54);
			
			while(1)
			{
				struct location loc4 = read_xy();
				if(loc4.x1>700 && loc4.y1>440)
				{
					num=3;
					show_bmp_quickly(0,0,800,480,"other.bmp"); 
					break;
				}
			}
			
		  }
		  
		  if(loc3.x1>400 && loc3.y1>100 && loc3.y1<440)
		  {
			  num=5;
			  show_bmp_quickly(0,0,800,480,"dog.bmp");
			  write(tcp_socket,"GetNewsMsg\r\n\r\n",strlen("GetInstantMsg\r\n\r\n"));
			  input_word2(rizhi,50,0,100,800,300);
			  while(1)
			  {
				struct location loc5 = read_xy();  
				if(loc5.x1>700 && loc5.y1>440)
				{
					num=3;
					show_bmp_quickly(0,0,800,480,"other.bmp"); 
					break;
				}
			  }
		
		
		  }
		  
		  if(loc3.x1>400 && loc3.y1>440)
		  {
			show_bmp_quickly(0,0,800,480,"chat.bmp");
			break;
		  }
	 
	    }   
	 
	 


	

 }
}
}

//滑动判断结构体
struct location read_xy()
{
	struct location a;
	int flag=1,flag1=1,x,y;


	//初始化输入子系统结构体	
	struct input_event event;	
    bzero(&event,sizeof(struct input_event));
	int event_fd = open("/dev/input/event0",O_RDWR);//打开触摸屏
	if (event_fd == -1)
	{
		printf("%s\n", "open event0 failed!");
		
	}
	while(1)
	{
		read(event_fd,&event,sizeof(struct input_event));	//读取触摸屏数据放到event结构体里
		if(event.type == EV_ABS && event.code == ABS_X)	//x坐标
		{
			x=event.value;
#if  TS_MODE
			x=x*800/1024;
#endif			
			if(flag==1)    //初始值通过中间变量x保存到x1
			{
				a.x1 =x;
				flag =0;
			}
					
		}	
			
		else if(event.type == EV_ABS && event.code == ABS_Y)//y坐标
		{
			y=event.value;
#if  TS_MODE
			y = y*480/600;
#endif			
			if(flag1==1)    //初始值通过中间变量y保存到y1
			{
				a.y1 = y;
				flag1 =0;
			}
	
		}	
		if (event.type == EV_KEY && event.code == BTN_TOUCH && event.value==0)
		{	
			a.x2=x;
			a.y2=y;
			break;
		}
			
	}
	printf("x1:%d\n",a.x1);
	printf("y1:%d\n",a.y1);
	printf("x2:%d\n",a.x2);
	printf("y2:%d\n",a.y2);
	
	
	if((abs(a.x2-a.x1))<5 && (abs(a.y2-a.y1))<5)
	{
		a.type=0;
		printf("单击\n");
	}
	
	else if(a.x2>(a.x1+50) ) 
	{
		a.type=1;
		printf("右滑\n");
		a.right_move=a.x2-a.x1;
	}
	
	else if(a.x1>(a.x2+50))
	{
		a.type=2;
		printf("左滑\n");
		a.left_move=a.x1-a.x2;
	}
	
	else if(a.y2>(a.y1+50) )
	{
		a.type=3;
		printf("下滑\n");
	}
	
	else if(a.y1>(a.y2+50))
	{
		a.type=4;
		printf("上滑\n");
	}
	return a;
}

int show_bmp_quickly(int x, int y, int bmp_w, int bmp_h,char *bmp_name) //快速显示图片
{
	//判断是否会图片太大
	if(bmp_h+y > 480 || x+bmp_w > 800)
	{
		printf("bmp too big!\n");
		return -1;
	}
	
	
	// 1》打开bmp文件 400*300
	FILE *fp = fopen(bmp_name, "r");
	if(fp == NULL)
	{
		//printf("fopen %s failed!\n", bmp_name);
		//perror("");
		fprintf(stderr, "fopen %s failed: %s", bmp_name, strerror(errno));
	}
	
	// 2》跳过54个字节文件头
	lseek(fp->_fileno, 54, SEEK_SET);

	
	unsigned char bmp_buf[bmp_w*bmp_h*3];
	bzero(bmp_buf, bmp_w*bmp_h*3);
	// 3》读取bmp文件的数据 bmp_w*bmp_h*3
	fread(bmp_buf, bmp_w*bmp_h*3, 1, fp);
	
	// 4》关闭文件
	fclose(fp);
	

	
	unsigned int lcd_buf[bmp_w*bmp_h];
	bzero(lcd_buf, bmp_w*bmp_h*4);
	// 5》合成LCD类型像素点数据
	//合成bmp_h排像素点

	int i, j;
	for(j=0; j<bmp_h; j++)
	{
		for(i=0; i<bmp_w; i++)
		{
			
			lcd_buf[j*bmp_w+i] = (bmp_buf[3*i+3*j*bmp_w+0]<<0) | (bmp_buf[3*i+3*j*bmp_w+1]<<8) | (bmp_buf[3*i+3*j*bmp_w+2]<<16) ;
			
		}

	}

	
	// 6》打开LCD屏幕文件
	int lcd_fd = open("/dev/fb0", O_RDWR);
	if(lcd_fd == -1)
	{
		perror("open /dev/fb0 failed!");
		return -1;
	}
	
	// 7》内存映射
	unsigned int *addr = mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
	if(addr == MAP_FAILED)
	{
		perror("mmap failed!");
		close(lcd_fd);
		return -1;
	}
	//int x=200, y=100;

	// 8》把数据通过指针赋值的方式拷贝到内存

	for(i=0; i<bmp_w; i++)
	{
		for(j=0; j<bmp_h; j++)
		{
			//j=0,i=0, 800*0+0
			//j=0,i=1, 800*0+1
			
			//i=0, j=0, 800*0+0 ==0
			//i=0, j=1, 800*1+0 ==800
			//i=0, j=2, 800*2+0 == 1600
			
			*(addr+800*j+i+800*y+x) = lcd_buf[(bmp_h-1-j)*bmp_w+i]; //正立，非原点
		}
		
		
	}
	
	// 9》关闭文件和解除映射
	close(lcd_fd);
	munmap(addr, 800*480*4);
	return 0;
	
}


void input_word2(char *name,int word_size,int xh,int yz,int k_wide,int k_hight)//输出框x,y坐标、长和宽
{
	
	
	//1.初始化字库
	font *tp = fontLoad("/usr/share/fonts/DroidSansFallback.ttf");
	
	//设置字体的大小
	fontSetSize(tp,word_size);
	
	                            //getColor(a, b, c, d) A B   G R  
	bitmap *bp=createBitmapWithInit(k_wide,k_hight,4,getColor(255,0,0,0));  //指定颜色
	
	//把字体填充到输出框中 
	fontPrint(tp,bp,0,0,name,getColor(0,255,255,255),k_wide);//最后一个参数：是字体大小的倍数换行
									//A B G  R 

	
	//把字体框输出到 LCD 设备中
	show_font_to_lcd(lcd_fd,xh,yz,bp);
	
	
		
	fontUnload(tp);
	destroyBitmap(bp);
	
}

int lcd_init()
{
	int fd=open("/dev/fb0",O_RDWR);   //打开lcd
	if(fd==-1)
	{
		perror("");	
		return -1;
	}
	printf("open lcd ok\n");
	//映射lcd设备
	lcd_fd=mmap(NULL,800*480*4,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
	if(lcd_fd==MAP_FAILED)
	{
		perror("");
		return -1;
	}
	printf("mmap ok\n");
}