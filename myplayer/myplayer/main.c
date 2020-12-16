#include "myplayer.h"
//#define DEBUG
#define MAIN
//#define TEST



#define HOME "/root/wallpaper/home.jpg"
linknode home;
int size=0;//定义一个存储现有链表节点个数的全局变量
int buf_size=0;//定义一个存储图片现有缓冲区个数的全局变量
linklist head;//将头节点定义到全局变量，给所有线程共享！
thread_pool *pool;//同理，共享线程池
Point p_all;
char *percent_v;//当前视频的文件名
FILE* mp;//接收mplayer的管道信息


//定义一个线程间通信的标志位来决定线程的阻塞与执行状态 采用8421码  8  4  2  1 分别对应线程4 3 2 1   1为主线程！
//如0x01线程1执行，其他进程阻塞
int chat_tid=0;
int Bar_P=0;//拖动进度条标志位
int Bar_act=0;//进度条交互动作  拖动/单击
int  timepos=0;

/*
制作一个视频播放器

    视频目录：     /root/myplayer/video
    预览图生成目录：/root/myplayer/video_pic
    截图的缓冲目录：/root/myplayer/buf_pic（自动创建）
    生成的预览图固定大小为： 250×150的JPG图片

*/

void print_node(linklist p )
{
    switch(p->num_preview)
    {
        case 1 :showPreview(p,100,50);
                break;

        case 2 :showPreview(p,450,50);
                break;

        case 3 :showPreview(p,100,280);
                break;

        case 4 :showPreview(p,450,280);
                break;

        default: break;

    }
    printf("\n");
    printf("视频名：%s\n",p->vname);
    printf("预览图名：%s\n",p->dname);
    printf("预览图所属块：%d\n", p->num_preview);
    printf("size r_w:%d ,r_h:%d：\n", p->rgb_w,p->rgb_h);
    printf("\n");

}


/***播放视频*****/
void *video(void *arg)
{
    /*
    int test=1;
    if(test ==1)
    {
             int i= -129;

        while(1)
        {
             system("killall -9 mplayer");
             scanf("%d",&a);
            printf("行号：%d\n",__LINE__);
        pos=Find_name(head,percent_v);
          printf("行号：%d\n",__LINE__);
        pos=pos->next;
        if(pos == &head->list)
                 pos=pos->next;

        p=list_entry(pos,struct node,list);
          printf("行号：%d\n",__LINE__);
        percent_v=p->vname;//更新当前视频名字
        bzero(buf_sys,sizeof(buf_sys));
        sprintf(buf_sys,"mplayer -quiet -zoom -x 800 -y 350 '%s'",percent_v);
        system(buf_sys);
        printf("当前视频：%s\n",percent_v);


         }





        while(1)
        {
            ;
        }
    }

    */


//printf("进入线程2！\n");
       //showRgb(NULL,0,0,0);
       percent_v=arg;//获取主线程传来的视频名
       /***刷功能块****/
       linklist bar=malloc(sizeof(linknode));
       bzero(bar,sizeof(linknode));
        strcpy(bar->dname,"/root/bar.jpg");
         showBar(bar,0,350,8);

        linklist bar_pause=malloc(sizeof(linknode));
        bzero(bar_pause,sizeof(linknode));
         strcpy(bar_pause->dname,"/root/Bar_pause.jpg");

        /***刷功能块****/

      while(1)
      {
              char buf[1000]={0};
              char buf_sys[1000]={0};
              int fd;
              struct list_head *pos;
              linklist p;
              int zoom_flag=0;
              int pause_flag=0;
              //float timepos;
              if(access("/root/pipe",F_OK))
              {
                  mkfifo("/root/pipe",0777);
              }
              fd=open("/root/pipe",O_RDWR);
              if(fd <0)
              {
                  printf("打开管道文件失败！\n");
              }

              sprintf(buf,"mplayer -quiet -slave -input file=/root/pipe -zoom -x 800 -y 350 '%s'",percent_v);
              printf("准备打开mplayer!\n");
               mp=popen(buf,"r");//mp指向stdout
              //system(buf);/**使用system阻塞了？***/
              // int  type_flag=-1;//决定是否清理进度条
               add_task(pool,task,NULL);
              chat_tid= 1;//让线程3去执行刷进度条的任务
              sleep(1);


              while(1)
              {
                  p_all=GetPoint();

                  if(p_all.dx == 1)//拖动进度条右移动
                  {
                      Bar_act=1;
                      Bar_P =p_all.sum_x;

                  }


                 else if(p_all.dx == -1)
                  {
                      Bar_act=1;
                      Bar_P = p_all.sum_x;


                  }

                  else if( p_all.Y>353 && p_all.Y<380 && p_all.dx==0&& p_all.dy==0)
                  {

                      Bar_act=2;
                      Bar_P = p_all.X;

                  }


                  else if(p_all.X>350 && p_all.X<450  && p_all.Y>400 && p_all.Y<480)//开始/暂停
                  {


                      if(pause_flag  ==0)
                      {
                          showBar(bar_pause,0,350,8);
                          system( "killall -19 mplayer" );
                          pause_flag=1;


                      }
                      /****注意这里一定是else if  选其一！！！！*****/
                      else if(pause_flag ==1)
                      {
                           showBar(bar,0,350,8);
                          system("killall -18  mplayer");
                          pause_flag=0;



                      }

                  }
                  else if(p_all.X>450 && p_all.X<550  && p_all.Y>400 && p_all.Y<480)//下一个  右
                  {

                      system("killall -9 mplayer");
                      /*****这里选择关闭mp后 ，会出现段错误！*****/
                     //pclose(mp);


                      pos=Find_name(head,percent_v);
                      pos=pos->next;
                      if(pos == &head->list)
                               pos=pos->next;

                      p=list_entry(pos,struct node,list);
                      percent_v=p->vname;//更新当前视频名字
                      bzero(buf_sys,sizeof(buf_sys));
                      sprintf(buf_sys,"mplayer -quiet -slave -input file=/root/pipe -zoom -x 800 -y 350 '%s'",percent_v);

                      mp=popen(buf_sys,"r");

                      /***这里使用system打开依然阻塞！！！*****/


                  }
                  else if(p_all.X>250 && p_all.X<350  && p_all.Y>400 && p_all.Y<480)//上一个  左
                  {

                      system("killall -9 mplayer");
                      //pclose(mp);


                      pos=Find_name(head,percent_v);
                      pos=pos->prev;
                      if(pos == &head->list)
                               pos=pos->prev;

                      p=list_entry(pos,struct node,list);
                      percent_v=p->vname;//更新当前视频名字


                      bzero(buf_sys,sizeof(buf_sys));
                      sprintf(buf_sys,"mplayer -quiet -slave -input file=/root/pipe -zoom -x 800 -y 350 '%s'",percent_v);
                      mp=popen(buf_sys,"r");

                      /***这里使用system打开依然阻塞！！！*****/



                  }
                  else if(p_all.X>550 && p_all.X<650  && p_all.Y>400 && p_all.Y<480)//快进   右
                  {
                      system("echo \"seek +10\" > /root/pipe");


                  }
                  else if(p_all.X>150 && p_all.X<250  && p_all.Y>400 && p_all.Y<480)//倒退  左
                  {
                      system("echo \"seek -10\" > /root/pipe");

                  }
                  else if(p_all.X>650 && p_all.X<725  && p_all.Y>400 && p_all.Y<480)//声音+
                  {
                      system("echo \"volume 10\" > /root/pipe");

                  }
                  else if(p_all.X>725 && p_all.X<800  && p_all.Y>400 && p_all.Y<480)//声音-
                  {
                       system("echo \"volume -10\" > /root/pipe");

                  }
                  else if(p_all.X>0 && p_all.X<150  && p_all.Y>400 && p_all.Y<480)//全屏
                  {



                      if(zoom_flag ==0)
                      {
                          system("killall -9 mplayer");

                          int tmp_time=timepos;


                          bzero(buf_sys,sizeof(buf_sys));
                          sprintf(buf_sys,"mplayer -quiet -slave -input file=/root/pipe -zoom -x 800 -y 480 '%s'",percent_v);
                          mp=popen(buf_sys,"r");


                          bzero(buf,1000);
                          sprintf(buf,"echo \"seek %d 1 \" > /root/pipe",tmp_time);
                          system(buf);

                          zoom_flag=1;
                          chat_tid=0;
                      }

                      else if(zoom_flag ==1)
                      {
                          system("killall -9 mplayer");
                             int tmp_time=timepos;

                          bzero(buf_sys,sizeof(buf_sys));
                          sprintf(buf_sys,"mplayer -quiet -slave -input file=/root/pipe -zoom -x 800 -y 350 '%s'",percent_v);
                          mp=popen(buf_sys,"r");


                          bzero(buf,1000);
                          sprintf(buf,"echo \"seek %d 1 \" > /root/pipe",tmp_time);
                          system(buf);


                           showBar(bar,0,350,8);
                          zoom_flag=0;
                          chat_tid=1;

                      }


                  }

                  //在右下角垂直向上滑动表示退出！
                  else if((p_all.dx==0&&p_all.dy==-1)&&(p_all.X>=550&&p_all.Y>=200))
                  {
                       chat_tid=-1;//-1为线程的退出指令！
                       system("killall -9 mplayer");
                       showRgb(NULL,0,0,0);
                      printf("播放界面已退出！\n");


                      //这里等待线程结束
                      while(1)
                      {
                          if(chat_tid ==-2)//这里规定线程向chat_tid 写-2表示线程已经结束！
                          {
                               return 0;

                          }

                      }



                  }



              }
          }








}



/*****实时更新进度条 ，进度条范围： Y：（350,380）***/
void *task(void *arg)
{
    while(1)
    {
        if( chat_tid ==1)//收到执行信号
        {
            printf("进度条开始执行！ %d\n",Bar_P);
            int fd =open("/dev/fb0",O_RDWR);//注意这里是内存映射，应该读写
            if(fd ==-1)
            {
                perror("open fail!");
            }



            //3,获取LCD设备底层信息（即屏幕的尺寸、色深等信息）
            struct fb_var_screeninfo vinfo;//存放LCD屏幕信息的结构体
            bzero(&vinfo,sizeof(struct fb_var_screeninfo));
            ioctl(fd,FBIOGET_VSCREENINFO,&vinfo);//获取LCD的尺寸等的相关信息
            //根据实际获取的LCD的信息定义LCD的分辨率及其虚拟区的分辨率
            //LCD的分辨率及其色深（可见区）
            int w = vinfo.xres; // 800
            int h = vinfo.yres; //480
            int bpp = vinfo.bits_per_pixel;// 32
            //虚拟区的分辨率
            int virtual_w=vinfo.xres_virtual;
            int virtual_h=vinfo.yres_virtual;

            //4,将LCD设备映射到一块恰当大小的内存中
            char *p0=mmap(NULL,virtual_h*virtual_w*(bpp/8),PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);

             char buf[1000]={0};
             int rgb_h=30;
             int rgb_w;

             int origin_x=0;
             int origin_y=353;
             int tpy=0x00;
             timepos=0;
             char blue[3]={0xff,0x00,0x00};
             char white[3]={0xff,0xff,0xff};


             sleep(1);
             //实时刷图！
             while(1)
             {
                 if(chat_tid ==1)
                 {


                     if(Bar_P == 0  )
                     {

                         system("echo \"get_percent_pos\" > /root/pipe");
                         bzero(buf,1000);
                         fgets(buf, 1000, mp);

                         /****sscanf筛选作用有奇效！****/
                         sscanf(buf,"ANS_PERCENT_POSITION=%d",&timepos);



                        rgb_w=(int) (800 * timepos * 0.01);//实时获取当前播放进度
                        for(int i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                        {
                            long offset=(i+origin_y)*w*4;	//LCD行的偏移量
                            for(int j=0;j<(int)rgb_w;j++)//一行有info.biWidth个像素点
                            {
                                /*rgb_w*3*i为RGB的行偏移量*/
                                memcpy(p0+4*j+offset+(origin_x)*4,blue,3);
                                memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                            }
                        }

                        for(int i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                        {
                            long offset=(i+origin_y)*w*4;	//LCD行的偏移量
                            for(int j=rgb_w;j<(int)800;j++)//一行有info.biWidth个像素点
                            {
                                /*rgb_w*3*i为RGB的行偏移量*/
                                memcpy(p0+4*j+offset+(origin_x)*4,white,3);
                                memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                            }
                        }



                     }


                    if(Bar_P != 0  && Bar_act==1 )//拖动
                    {




                            int time=(timepos+(int)(((float)Bar_P/800)*100));
                            if(time >=99)
                            {
                                time=97;
                            }

                            rgb_w =(int) ((800) *(int)time * 0.01);
                            if(rgb_w >= 800)
                            {
                                rgb_w=780;
                            }


                            bzero(buf,1000);
                            sprintf(buf,"echo \"seek %d 1 \" > /root/pipe",(int)time);
                            system(buf);
                            for(int i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                            {
                                long offset=(i+origin_y)*w*4;	//LCD行的偏移量
                                for(int j=0;j<(int)rgb_w;j++)//一行有info.biWidth个像素点
                                {
                                    /*rgb_w*3*i为RGB的行偏移量*/
                                    memcpy(p0+4*j+offset+(origin_x)*4,blue,3);
                                    memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                                }
                            }

                            for(int i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                            {
                                long offset=(i+origin_y)*w*4;	//LCD行的偏移量
                                for(int j=rgb_w;j<(int)800;j++)//一行有info.biWidth个像素点
                                {
                                    /*rgb_w*3*i为RGB的行偏移量*/
                                    memcpy(p0+4*j+offset+(origin_x)*4,white,3);
                                    memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                                }
                            }


                        Bar_P =0;
                        Bar_act=0;




                    }

                    if(Bar_P != 0  && Bar_act==2   )//单击
                    {

                        int time=((int)(((float)Bar_P/800)*100));
                        if(time >=99)
                        {
                            time=97;
                        }

                        rgb_w =(int) ((800) *(int)time * 0.01);
                        if(rgb_w >= 800)
                        {
                            rgb_w=780;
                        }

                        bzero(buf,1000);
                        sprintf(buf,"echo \"seek %d 1 \" > /root/pipe",(int)time);
                        system(buf);
                        //printf("time:%d\n",time);
                        for(int i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                        {
                            long offset=(i+origin_y)*w*4;	//LCD行的偏移量
                            for(int j=0;j<(int)rgb_w;j++)//一行有info.biWidth个像素点
                            {
                                /*rgb_w*3*i为RGB的行偏移量*/
                                memcpy(p0+4*j+offset+(origin_x)*4,blue,3);
                                memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                            }
                        }

                        for(int i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                        {
                            long offset=(i+origin_y)*w*4;	//LCD行的偏移量
                            for(int j=rgb_w;j<(int)800;j++)//一行有info.biWidth个像素点
                            {
                                /*rgb_w*3*i为RGB的行偏移量*/
                                memcpy(p0+4*j+offset+(origin_x)*4,white,3);
                                memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                            }
                        }


                       Bar_P =0;
                       Bar_act=0;



                    }

                 }
                 else if(chat_tid == -1)
                 {

                     for(int i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                     {
                         long offset=(i+origin_y)*w*4;	//LCD行的偏移量
                         for(int j=0;j<(int)800;j++)//一行有info.biWidth个像素点
                         {

                             memcpy(p0+4*j+offset+(origin_x)*4,white,3);
                             memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                         }
                     }

                     break;

                 }
                 else
                 {
                     ;
                 }



             }


        }

        else if(chat_tid ==-1)
        {
            chat_tid=-2;
            break;
        }
        else
        {
            ;
        }




    }



}


void Interface(linklist head)
{
    //搞一个单独的节点放首页照片，与链表分离
    Point ph;
    strcpy(home.dname,HOME);
     strcpy(home.format,"jpg");
    home.flag=0;
    showPicture(&home,1);//显示

    //1,获取坐标
    ph=GetPoint();
    while(1)
    {
        //单击行为且捕捉到进入按钮
        if(ph.flag<=4&&(ph.X>=300&&ph.X<=700)&&(ph.Y>=165&&ph.Y<=400))
        {
            PreView(head);
            showPicture(&home,1);//显示
            ph=GetPoint();//再次进行触摸判断，不触摸时为阻塞状态
        }

        //在右下角垂直向上滑动表示退出！
        else if((ph.dx==0&&ph.dy==-1)&&(ph.X>=550&&ph.Y>=200))
        {
            printf("预览图界面已退出！\n");

            return ;
        }

        //捕捉到非法区域
        else
        {
            ph=GetPoint();//再次进行触摸判断，不触摸时为阻塞状态
        }




    }

}

//影片的预览图

/*

向右：

1）初始状态下，默认右滑动显示第一副预览图

2）够4张，tmp保存初始指针，pos指向新的节点！下一次

3）不够4张，即到了尾节点


*/





int PreView(linklist head)
{


    struct list_head *pos;//指向小结构体的指针
    struct list_head *n;
    Point point;
    linklist p;
    char *(p_view[100])[4];//创建一个数组，存放100个元素，元素类型为char *[4]，用于存放4个预览图的文件名指针！
    bzero(p_view,sizeof(p));
    int i=0;
    int num=0;//表示预览图的个数！  0->小于4张


    //遍历链表，以1张4个预览图为单位将每张图片缓冲到数组中！
    list_for_each_safe(pos,n,&((head)->list))
    {
        linklist p=list_entry(pos,struct node,list);
        switch(p->num_preview)
        {
            case 1:
                    p_view[num][i++]=p->dname;
                    break;
            case 2:
                    p_view[num][i++]=p->dname;
                    break;
            case 3:
                    p_view[num][i++]=p->dname;
                    break;
            case 4:
                   p_view[num][i++]=p->dname;
                    break;

            default:
                    break;
        }
        if(i ==5)
        {
            i=0;
            num++;
        }

    }

    for(int i=0;;)
    {
        if(i < 0)
        {
            i=0;
        }

        if(i > num)
        {
            i=num;
        }

        //显示第一张！
        showRgb(NULL,0,0,0);
        linknode *p_tmp[4];
        bzero(p_tmp,sizeof(p_tmp));

        p_tmp[0]=Find_dname(head,p_view[i][0]);
        showPreview( p_tmp[0],100,50);

        p_tmp[1]=Find_dname(head,p_view[i][1]);
        showPreview(p_tmp[1],450,50);

        p_tmp[2]=Find_dname(head,p_view[i][2]);
        showPreview(p_tmp[2],100,280);

        p_tmp[3]=Find_dname(head,p_view[i][3]);
        showPreview(p_tmp[3],450,280);



        while(1)
        {
            point=GetPoint();
            //水平向右滑动！
            if(point.dx==1&&point.dy==0)
            {
                i--;
                break;

            }

            //水平向左滑动
            else if(point.dx==-1&&point.dy==0)
            {
               i++;
               break;

            }
            /*****播放视频***************/
            else if(point.X>100 && point.X<350  && point.Y>50 && point.Y<200 )//1
            {
                 /***这里判断非空，防止误操作导致段错误！****/
                if(p_tmp[0] !=NULL)
                {

                    video(p_tmp[0]->vname);
                    break;


                }
            }

            else if(point.X>500 && point.X<750  && point.Y>50 && point.Y<200)//2
            {

                if(p_tmp[1] !=NULL)
                {

                    video(p_tmp[1]->vname);
                    break;


                }


            }
            else if(point.X>100 && point.X<350  && point.Y>280 && point.Y<430)//3
            {
                if(p_tmp[2] !=NULL)
                {

                    video(p_tmp[2]->vname);
                    break;


                }

            }
            else if(point.X>500 && point.X<750  && point.Y>280 && point.Y<430)//4
            {
                if(p_tmp[3] !=NULL)
                {

                    video(p_tmp[3]->vname);
                    break;


                }

            }

            //在右下角垂直向上滑动表示退出！
            else if((point.dx==0&&point.dy==-1)&&(point.X>=550&&point.Y>=200))
            {
                printf("预览图界面已退出！\n");

                return 0;
            }

         }




        }

 }








int main()
{

#ifdef TEST

    showRgb(NULL,0,0,0);

    FILE* mp;
    char buf[1000]={0};
    char buf_in[1000]={0};

    if(access("/root/pipe",F_OK))
    {
        mkfifo("/root/pipe",0777);
    }
    int fd=open("/root/pipe",O_RDWR);
    if(fd <0)
    {
        printf("打开管道文件失败！\n");
    }

    mp=popen("mplayer -quiet -slave -input file=/root/pipe '/root/myplayer/video/ssjs.avi'","r");
    system("echo \"get_percent_pos\" > /root/pipe");
     int num=0;
     while(1)
     {

         bzero(buf,1000);
         fgets(buf, 1000, mp);
         sscanf(buf,"ANS_PERCENT_POSITION=%d",&timepos);
         printf("timepos:%d\n",timepos);

     }







#endif




#ifdef MAIN
    //1,初始化参数！

    //1）线程池初始化
        pool=(thread_pool *)malloc(sizeof(thread_pool));
        //初始化线程池
        init_pool(pool, 10);
        sleep(1);



    //2）预览图初始化，截取预览图并加入到链表中
       head=init_list();
       printf("初始化成功！\n");
       system("killall -9 mplayer");
       showRgb(NULL,0,0,0);
       Get_Vpic(head);
       if(access(HOME ,F_OK))
        {
           printf("首页图不存在！\n");
           return 0;
        }


#ifdef  DEBUG
 showRgb(NULL,0,0,0);//黑屏！
 travel_list(head,print_node);
 printf("当前的链表图片个数：%d\n",size);
 // showRgb(NULL,0,0,0);//黑屏！
#endif



       Interface(head);
       showRgb(NULL,0,0,0);
       destroy_pool(pool);
       list_destroy(&head);
       return 0;


#endif

}
