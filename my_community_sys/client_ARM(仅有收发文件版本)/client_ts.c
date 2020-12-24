#include <stdio.h>
#include <netinet/in.h>   //for souockaddr_in
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h> 
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <strings.h>   //for bzero
#include <string.h>
#include <netinet/ip.h>
#include <pthread.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/input.h>
#include <linux/fb.h>//存放相关设备的说明书
#include <sys/mman.h>

#define MAIN_CLIENT

#define  TS_MODE  1

int tcp_socket;

char *name;
char *house;

struct usr 
{
	char usr_addr_buf[100];
	char usr_name[50];
	int usr_sokect;
};

char  file_num=0;

/*供发送文件子线程使用的全局变量*/
int file_Send_flag=0;//开启发送文件线程的标志位
char path[100]={0};//发送的文件名
char read_name[100]={0};//文件的接受者

/*接受文件*/

#define INFO_dir  "/root/usr_dir"//存放接受的文件的目录！


int Send_info_flag=0;//判断是否可以发送文件的原始数据了！
int ts_fd;
struct weather w;//存储当前最新的天气信息
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

int i=0;
struct location read_xy();
int t();

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



int set_bmp(int bmp_w,int bmp_h,int m,int n,const char *bmp_name,int c);
int show_black();

void *file_Send(void *arg);
int file_Recv(const char *path,unsigned int fileSize);






void *select_func(void *arg)
{
	struct usr * p=arg;
	char personal_information1[500]={0};
	char new_name[50]={0};
	char write_buf[1024]={0};
	fd_set set;
    int max_fd=1;
    struct timeval t;
	
	while(1)
	{
		//struct location loc = read_xy();
		
		
		t.tv_sec=27;
        t.tv_usec=0;
 
        FD_ZERO(&set);
 
        //add standard input
        FD_SET(0,&set);
        if(max_fd < 0)
        {
            max_fd=0;
        }
		
        //add serverce
        FD_SET(p->usr_sokect,&set);
        if(max_fd < p->usr_sokect)
        {
            max_fd = p->usr_sokect;
        }
		//select多路复用
        int ret = select(max_fd + 1, &set, NULL, NULL, &t);
 
        if(ret < 0)   
        {   
            perror("select failure\n");   
            continue;   
        }   
 
        else if(ret == 0)
        {
            printf("time out!");
            continue;
        }
 
        else
        {
			if(FD_ISSET(p->usr_sokect,&set))
			{
					
					char read_buf[4096]={0};
					read(p->usr_sokect,read_buf,1024);
				
				if(strstr(read_buf,"read_msg"))//接收其他用户消息
				{ 
					printf("!!!!\n");
					printf("%s\n",read_buf);
				}
				if(strstr(read_buf,"RecvInstantMsg"))//接收天气信息
				{
					//w为天气结构体，全局变量，自动更新最新的天气！
					
					bzero(&w,sizeof(w));
					memcpy((char *)&(w),read_buf+strlen("RecvInstantMsg\r\n"),sizeof(w));
					
					
						
					//test:	
					
					 printf("date:%s    week:%s   city:%s\n",w.date,w.week,w.city);
					 printf("wea:%s    tem:%s   tem1:%s    tem2:%s\n",w.wea,w.tem,w.H_tem,w.L_tem);
					 printf("win:%s    win_speed:%s   humidity:%s\n",w.win,w.win_speed,w.humidity);
					 printf("air:%s    air_level:%s   air_tips:%s\n",w.air,w.air_L,w.air_tips);
				}
				if(strstr(read_buf,"SendNotice\r\n"))//接收公告
				{
					char back_buf[100]={0};
					strcpy(back_buf,"usrRECVnotice\r\n\r\n");
					printf("%s\n",read_buf + strlen("SendNotice\r\n"));
					write(p->usr_sokect,back_buf,sizeof(back_buf));
					
				}
				if(strstr(read_buf,"loin\r\n\r\n"))
				{
					//printf("2,%s\n",p->usr_addr_buf);
					sprintf(personal_information1,"usr_msg:\r\n[name:%s]\n[house:%s]\r\n\r\n",p->usr_name,p->usr_addr_buf);
					write(p->usr_sokect,personal_information1,sizeof(personal_information1));
					printf("已回发个人信息给主机！\n");
				}
				if(strstr(read_buf,"Send2usr:"))//接收文件
				{
					int choose=0;
					printf("wether you accept the file or not\n");
					printf("1.reject   2.accept\n");
					scanf("%d",&choose);
					if(choose == 1)
					{
						char answer_buf[100]={0};
						stpcpy(answer_buf,"reject the info!\r\n\r\n");
						write(p->usr_sokect,answer_buf,sizeof(answer_buf));
						break;
					}
					if(choose == 2)
					{
						write(p->usr_sokect,"accept the info!\r\n\r\n",strlen("accept the info!\r\n\r\n"));
						char path[50]={0};//文件名
						char non1[50]={0};
						char non2[50]={0};
						char write_buf[50];
						int fileSize=0;
						printf("read_buf=%s\n",read_buf);
						//Send2usr: [%s] [%s] [%d]
						sscanf(read_buf,"%s %s %s %d",non1,write_buf,path,&fileSize);
						printf("path=%s,file_size=%d\n",path,fileSize);
						file_num++;
						chdir(INFO_dir);//进入到文件存放目录
						file_Recv(path,fileSize);
					}
				}
				
				
				//发送文件的标志，收到服务器传来的回文！
				if(strstr(read_buf,"please2SendInfo"))
				{
					printf("please2SendInfo\n");
					Send_info_flag=1;
				}
				
				
				
				if(strstr(read_buf,"usr not exist or has outline\r\n\r\n"))
				{
					printf("usr not exist or has outline!\n");
				}
			}
			
			
			if(FD_ISSET(0,&set)) //标准输入是否存在于ser_fdset集合中（也就是说，检测到输入时，做如下事情）
            {
                int a=0;
				scanf("%d",&a);
				if(a == 1)
					
				{
					
					char information[4096]={0};
					char send_buf[500]={0};
					//strcpy(write_buf,"\r\n[Shady][information][read_usr]\r\n\r\n");
					printf("send to ...\n");
					scanf("%s",send_buf);
					printf("what you want to say?\n");
					scanf("%s",information);
					sprintf(write_buf,"write2usr:\r\nwrite=[%s]info=[%s]read=[%s]\r\n\r\n",p->usr_name,information,send_buf);
					printf("%s\n",write_buf);
					
		
					write(p->usr_sokect,write_buf, sizeof(write_buf));
				}
				if(a == 2)
				{
					printf("Asking for the weather...\n");
					char news_buf[1024]={0};
					stpcpy(news_buf,"GetInstantMsg\r\n\r\n");
					write(p->usr_sokect,news_buf, strlen(news_buf));
					printf("%s\n",news_buf);
				}
				if(a == 3)
				{
					printf("send file to...\n");
					scanf("%s",read_name);
					printf("Please enter the file name\n");
					scanf("%s",path);
					
					file_Send_flag=1;//开启发送文件的线程

				}
				
				
				if(a == 4)
				{
					printf("请输入向服务器测试的指令！\n");
					char buf[1024]={0};
					scanf("%s",buf);
					write(p->usr_sokect,buf, strlen(buf));
					printf("发送完毕！\n");

				}
					
            }
		}
		
	}
	
}

//例如：path:/home/gec/a.txt  ->严格按照这个格式，斜杠的数量
//   const char *path,const char *read_name
//创建一个单独的线程负责发送文件，若嵌套在select_func会发生阻塞！
void *file_Send(void *arg)
{
	struct usr *owner=arg;
	
	while(1)
	{
		if(file_Send_flag ==1)
	{
		char buf[10]={0};
		char file_buf[4096]={0};
		int size, netSize;
		unsigned int fileSize;
		FILE *fp = NULL;

		fp = fopen(path, "r");
		if( fp == NULL ) 
		{
		perror("fopen");

		close(owner->usr_sokect);
		return 0;
		}
		if( !path ) 
		{
		printf("file server: file path error!\n");
		return 0;
		}

		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char *file_name=strrchr(path,'/')+1;

		char info_ready[1024]={0};
		sprintf(info_ready,"transfer_info_ready: %s %d %s %s \r\n\r\n",name,fileSize,file_name,read_name);

		int ret_w =write(tcp_socket,info_ready,strlen(info_ready));//发送文件大小和文件名！
		if(ret_w <= 0)
		{
		perror("");
		}

		//printf("info_ready=%s   ,ret_w:%d\n",info_ready,ret_w);

		while(Send_info_flag == 0);//等待服务器获取文件大小和文件名！


		printf("服务器已经接受了文件大小和文件名\n");
		
		
		
		//首先发送文件头！
		strncpy(file_buf,"Info_Send:\r\n\r\n",strlen("Info_Send:\r\n\r\n"));
		netSize = write(owner->usr_sokect,file_buf,strlen(file_buf) );
		bzero(file_buf,sizeof(file_buf));
		printf("已发送：Info_Send\n");
		
		
		//这里的延时是非常必要的！原因是为了配合服务器第一次读到的数据仅是文件头，而不会读到额外的原始数据！！！！
		sleep(1);

		
		//发送文件的原始数据  
		int down_size=0;
		while(1) 
		{
			bzero(file_buf,sizeof(file_buf));
			//strncpy(file_buf,"Info_Send:\r\n\r\n",strlen("Info_Send:\r\n\r\n"));
			
			size = fread(file_buf, 1,4000, fp);
			down_size +=size;
			printf("down_size:%d\n",down_size);
			
			if(size <= 0 )
			{
				break;
			}
			unsigned int size2 = 0;
			
			while( size2 < size ) 
			{
				netSize = write(owner->usr_sokect, file_buf + size2, size - size2);
				if( netSize  < 0 ) 
				{
					perror("write");
				
					close(owner->usr_sokect);
					return 0;
				}
				size2 += netSize;
			}
			
		}
		
		printf("发送完毕！\n");
		file_Send_flag=0;//标志位置零  防止重复发送！！
		fclose(fp);

		
	}
	
		else
		{
			;
		}
	
		
	}
	
  
	
}




#ifdef MAIN_CLIENT

int main(int argc, char *argv[])
{
    //  ./main  name  192.168.22.xxx  端口
    if(argc !=5)
    {
        printf("./chat_client  name  house  192.168.22.xxx  端口\n");
	}  
	
	name=argv[1];
	house=argv[2];
	
	mkdir(INFO_dir,0777);//创建目录接受文件
	
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
	
	
	/* char usr_addr_buf[100]={0};
	char usr_name[50]={0}; */
	char personal_information[500]={0};
	char new_name[50]={0};
	struct usr owner;
	
	
	//test
	//strcpy(owner->usr_addr_buf,"A-201");
	//strcpy(owner->usr_name,"Shady");
	
	
	strcpy(owner.usr_addr_buf,argv[2]);
	strcpy(owner.usr_name,argv[1]);
	printf("%s\n",owner.usr_addr_buf);
	owner.usr_sokect = tcp_socket;
	/* strcpy(usr_addr_buf,"A-201");
	strcpy(usr_name,"Shady"); */
	//sprintf(personal_information,"usr_msg:\r\n[name:%s]\n[house:%s]\r\n\r\n",owner->usr_name,owner->usr_addr_buf);
	//write(tcp_socket,personal_information,sizeof(personal_information));
	//printf("已回发个人信息给主机！\n");
	//读取服务器发送过来的数据 
	
	pthread_t  pid;
	pthread_create(&pid,NULL,select_func,&owner);
	
	
	pthread_t  pid_Send;
	pthread_create(&pid_Send,NULL,file_Send,&owner);
	while(1);
	
	/*
	ts_fd = open("/dev/input/event0",O_RDONLY);
	set_bmp(800,480,0,0,"chat.bmp",1);
	
	/*
	//单次发送模板！
	char read_name[50]={0};
	char msg[1024]={0};
	printf("请输入接受者名字！\n");
	scanf("%s",read_name);
	printf("请输入消息！\n");
	scanf("%s",msg);
	Send2usr(msg,read_name);
	t();
	
	//关闭通信 
	//close(tcp_socket);
	*/

}

//发送文件函数



//接收文件函数
int file_Recv(const char *path,unsigned int fileSize)
{
	FILE *fp = NULL;
	unsigned int  fileSize2;
    int size, nodeSize;
	char buf[10]={0};
	char filerec_buf[4096]={0};
	

	
    fp = fopen(path, "w");
    if( fp == NULL ) 
	{
        perror("fopen");
        close(tcp_socket);
        return 0;
    }
	printf("打开成功！\n");
	
 
    fileSize2 = 0;
	sleep(1);//等待服务器发送原始数据
    while(memset(filerec_buf, 0, sizeof(filerec_buf)), (size = read(tcp_socket, filerec_buf, sizeof(filerec_buf))) > 0) 
	{
        unsigned int size2 = 0;
        while( size2 < size ) 
		{
            if( (nodeSize = fwrite(filerec_buf + size2, 1, size - size2, fp) ) < 0 ) 
			{
                perror("write");
                //close(tcp_socket);
                exit(1);
            }
            size2 += nodeSize;
        }
        fileSize2 += size;
		printf("fileSize2=%d\n",fileSize2);
        if(fileSize2 >= fileSize) 
		{
			                        //info accept success!\r\n\r\n
			write(tcp_socket,"info accept success!\r\n\r\n", strlen("info accept success!\r\n\r\n"));
			printf("接受完毕！\n");
            break;
        }
    }
    fclose(fp);

	
}


int t()
{
	ts_fd = open("/dev/input/event0",O_RDONLY);//访问触摸屏文件 
	
	if(ts_fd > 0)
		printf("open event0 ok!\n");
	
	struct input_event event;
	int ret,x,y,pressure;
	int count=0;
	int n = 0;
	
	
	printf("0");			
			
	while(1)
	{
		struct location loc = read_xy();
		
		if(50<loc.x1 && loc.x1 < 265 && 175 < loc.y1 && loc.y1 < 350)//视频
		{
			set_bmp(800,480,0,0,"chat1.bmp",1);

		}
		if(295<loc.x1 && loc.x1 < 515 && 175 < loc.y1 && loc.y1 < 350)//视频
		{
			set_bmp(800,480,0,0,"Notice.bmp",1);
			
			
		}
		if(555<loc.x1 && loc.x1 < 775 && 175 < loc.y1 && loc.y1 < 350)//视频
		{
			set_bmp(800,480,0,0,"other.bmp",1);
			
			
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


int set_bmp(int bmp_w,int bmp_h,int m,int n,const char *bmp_name,int c)
{
	
	// 1》打开bmp文件
	
	FILE *fp = fopen(bmp_name,"r");
	if(fp == NULL)
	{
		perror("bmp open failed!\n");
		return -1;
	}
	printf("showing %s \n", bmp_name);
	// 2》跳过54个字节文件头
	int ret=lseek(fp->_fileno, 54, SEEK_SET);
	if( ret == -1)
	{
		perror("lseek failed!\n");
		return -1;
	}
	else 
	{
		printf("ret: %d \n",ret);
	}
	
	int ret2 = (4-bmp_w*3%4)%4;
	
	// 3》读取bmp文件的数据
	unsigned char bmp_buf[bmp_h*bmp_w*3+ret2*bmp_h];//图片缓冲区
	bzero(bmp_buf, bmp_h*bmp_w*3+ret2*bmp_h);
	int ret1 = fread(bmp_buf, bmp_h*bmp_w*3+ret2*bmp_h, 1, fp);
	if(ret1 == 1)
	{
		printf("fread %d nmemb data!\n", ret1);
		
	}
	else if(ret1 < 1)
	{
		if(feof(fp))
		{
			printf("end !\n");
		}
		if(ferror(fp))
		{
			printf("error !\n");
		}
	}
	// 4》关闭文件
	fclose(fp);
	
	
	// 5》合成LCD类型像素点数据
	unsigned int lcd_buf[bmp_h*bmp_w];//屏幕缓冲区
	bzero(lcd_buf,bmp_h*bmp_w*4);
	
	int i, j;
	for(j=0; j<bmp_h; j++)
	{
		for(i=0; i<bmp_w; i++)
		{
		
			lcd_buf[j*bmp_w+i] = (bmp_buf[3*i+3*j*bmp_w+0]<<0) | (bmp_buf[3*i+3*j*bmp_w+1]<<8) | (bmp_buf[3*i+3*j*bmp_w+2]<<16) ;
			
		}
	}
	unsigned int show_buf[bmp_h*bmp_w];//上下颠倒
	bzero(show_buf,bmp_h*bmp_w*4);
	
	int x1,y1;
	for(y1=0;y1<bmp_h;y1++)
	{
		for(x1=0;x1<bmp_w;x1++)
		{
			show_buf[(bmp_h-1-y1)*bmp_w+x1]=lcd_buf[y1*bmp_w+x1];
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
	
	// 8》把数据通过指针赋值的方式拷贝到内存
	
	int a,b;
	
	if(c == 1)//从上往下
	{
		for(a=0; a<bmp_h; a++)
		{	
			for(b=0; b<bmp_w; b++)
			{
			
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
			}
			
		}
	}
	if(c == 2)//从左往右
	{
		for(b=0; b<bmp_w; b++)
		{
			for(a=0;a<bmp_h; a++)
			{
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
			}
			usleep(1000);
		}
		
	}
	if(c == 3)//从下往上
	{
		for(a=bmp_h-1; a>=0; a--)
		{	
			for(b=bmp_w-1; b>=0; b--)
			{
			
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
			}
			usleep(1000);
		}
	}
	if(c == 4)//从右往左
	{
		for(b=bmp_w-1; b>=0; b--)
		{
			for(a=bmp_h-1;a>=0; a--)
			{
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
			}
			usleep(1000);
		}
		
	}
	if(c == 5)//横向百叶窗
	{
		for(a=0; a<200; a++)
		{	
			for(b=0; b<480; b++)
			{
			
				*(addr+800*b+a+800*n+m) = show_buf[b*bmp_w+a];
				*(addr+800*b+a+200+800*n+m) = show_buf[b*bmp_w+a+200];
				*(addr+800*b+a+400+800*n+m) = show_buf[b*bmp_w+a+400];
				*(addr+800*b+a+600+800*n+m) = show_buf[b*bmp_w+a+600];
				
				
			}
			usleep(1000);
		}
	}
	if(c == 6)//纵向百叶窗
	{
		for(a=0; a<120; a++)
		{	
			for(b=0; b<800; b++)
			{
			
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
				*(addr+800*(a+120)+b+800*n+m) = show_buf[(a+120)*bmp_w+b];
				*(addr+800*(a+240)+b+800*n+m) = show_buf[(a+240)*bmp_w+b];
				*(addr+800*(a+360)+b+800*n+m) = show_buf[(a+360)*bmp_w+b];
				
				
			}
			usleep(1000);
		}
	}
	
	if(c == 7)//透明
	{
		show_black();
		int q,p;
		for(q=0; q<bmp_h; q++)
		{	
			if(q%2 != 0)
			{
				continue;
			}
			if(q%2 ==0)
			{
				for(p=0;p<bmp_w;p++)
				{
					if(p%2 != 0)
					{		
						continue;
					}
					if(p%2 ==0)
					{
					   *(addr+800*q+p+800*n+m) = show_buf[q*bmp_w+p];
					}
				}
			}
		}
	}
	
	if(c == 8)//缩小
	{
		int w,s,z;
		for(w=0,s=239;w<240;s--, w++)
		{
			for(z=0;z<400;z++)
			{		
				*(addr+800*w+z+800*n+m) = (bmp_buf[6*z+6*s*800+0]<<0) | (bmp_buf[6*z+6*s*800+1]<<8) | (bmp_buf[6*z+6*s*800+2]<<16) ;
			}
		}
	}		
	
	if(c == 9)//缩小x2
	{
		int w,s,z;
		for(w=0,s=119;w<120;s--, w++)
		{
			for(z=0;z<200;z++)
			{		
				*(addr+800*w+z+800*n+m) = (bmp_buf[12*z+12*s*800+0]<<0) | (bmp_buf[12*z+12*s*800+1]<<8) | (bmp_buf[12*z+12*s*800+2]<<16) ;
			}
		}
	}			
	
	if(c == 10)//从上往下
	{
		for(a=0; a<bmp_h; a++)
		{	
			for(b=0; b<bmp_w; b++)
			{
			
				*(addr+800*a+b+800*n+m) = show_buf[a*bmp_w+b];
			}
		}
	}	
	// 9》关闭文件和解除映射
	
	close(lcd_fd);
	munmap(addr, 800*480*4);
	printf("show %s success\n", bmp_name);
	return 0;
}


int show_black()//黑屏
{
	//2. 访问LCD液晶
	int fd;
	fd = open("/dev/fb0",O_RDWR);
	if(fd >= 0)
	{
		printf("open lcd ok!\n");
	}
	else
	{
		printf("open lcd error!\n");
	}
	
	unsigned int *addr = mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(addr == MAP_FAILED)
	{
		perror("mmap failed!");
		close(fd);
		return -1;
	}
	//2.5 填充颜色到LCD上。
	int color = 0x00000000;
	int i;
	for(i=0;i<800*480;i++)
	{
		*(addr+i)=color;
	}
	
	//3. 关闭LCD液晶
	int ret;
	ret = close(fd);
	if(ret == 0)
	{
		printf("close lcd ok!\n");
	}
	else{
		printf("close lcd error!\n");
	}
		
	return 0;						   //程序结束
}
#endif
