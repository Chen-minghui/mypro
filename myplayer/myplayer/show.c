#include "myplayer.h"

extern int size;//引用主函数的全局变量
extern  int buf_size;
char *  CmpFormat(char *dname)
{
    FILE *fn =fopen(dname,"r");
    if(fn==NULL)
    {
        perror("open fail");
        return NULL;
    }

    char buf[3]={0};//存储文件头三个字节来判断图片格式
    char *p=NULL;//返回图片信息的容器
    /*一段遗留下来的段错误：  类型：访问NULL造成的段错误*/
    //bzero(p,sizeof(linknode));
    //printf("行号：%d\n",__LINE__);

    int ret=fread(buf,1,3,fn);//读取三个字节
    if(ret==0)
    {
        ferror(fn);
    }

    if((buf[0]==(char)0xff)&&(buf[1]==(char)0xd8)&&(buf[2]==(char)0xff))
    {
        p="jpg";
    }

    else if(buf[0]==0x42&&buf[1]==0x4d)
    {
        p="bmp";
    }

    else
    {
         p="NULL";//未支持的图片格式
    }

    fclose(fn);

    return p;


}

Point GetPoint(void)
{
    Point p;
    bzero(&p,sizeof(Point));
#ifdef TEST
    printf("开始：flag=%d\n",p.flag);
#endif
    int StPoint_x;
    int StPoint_y;
    int fd =open("/dev/input/event0",O_RDONLY);//打开触摸屏文件
    if(fd == -1)
    {
        perror("open lcd fail");
        p.flag =-1;
        return p;
    }
    struct input_event buf;
    //3,读取触摸屏设备文件。相对非设备文件，有以下区别：
    //非设备文件：若文件数据为空或指向文件末尾，read返回0
    //设备文件：若文件数据为空或指向文件末尾，read处于阻塞状态！即实现等待读取
    while(1)
    {
        read(fd,&buf,sizeof(buf));

        if(buf.type == EV_ABS)//表示坐标信息事件类型
        {
            //判断该事件类型的具体信息
            if(buf.code == ABS_X)//x轴
            {
                p.flag++;
#ifdef TEST
    printf("中间：flag=%d\n",p.flag);
#endif
                p.X=800*buf.value/1024;
                if(p.flag ==1)
                {
                    StPoint_x=800*buf.value/1024;
                }

            }

            else if(buf.code == ABS_Y)//y轴
            {
                p.flag++;
#ifdef TEST
    printf("中间：flag=%d\n",p.flag);
#endif
                if(p.flag ==2)
                {
                    StPoint_y=800*buf.value/1024;
                }
                p.Y=480*buf.value/600;
            }


        }

        else if(buf.type == EV_KEY)//表示按键的事件类型，此时认为触摸屏整体为一个按键！
        {
            //判断该事件类型的具体信息

            if(buf.code == BTN_TOUCH&&buf.value == 0)//直到松手后才退出！
            {
                p.sum_x=p.X-StPoint_x;
               p. sum_y=p.Y-StPoint_y;
                if(p.X-StPoint_x >80)//判断X轴移动方向，80为一个区间误差！
                {
                    p.dx=1;//向右滑动
                }
                else if(p.X-StPoint_x <-80)
                {
                    p.dx=-1;//向左滑动
                }
                else
                {
                    p.dx=0;//无滑动
                }


                if(p.Y-StPoint_y >80)//判断Y轴移动方向
                {
                    p.dy=1;//向下滑动
                }
                else if(p.Y-StPoint_y < -80)//判断Y轴移动方向
                {
                    p.dy=-1;//向上滑动
                }
                else//判断Y轴移动方向
                {
                    p.dy=0;//无滑动
                }
                close(fd);
#ifdef TEST
    printf("\n");
    printf("结束：flag=%d\n",p.flag);
    printf("(%d,%d)\n",p.X,p.Y);
    printf("dx=%d,dy=%d\n",p.dx,p.dy);
    printf("\n");
#endif
                return p;//返回松手前最新的状态
            }
        }
    }

}

/*将BMP解码，同时将该BMP图片的分辨率传到链表数据中！失败返回NULL*/
char * Bmp2Rgb(linklist pinfo)
{
    //1,打开指定目录的bmp图片文件
    FILE *fn=fopen(pinfo->dname,"r");
    if(fn ==NULL)
    {

        perror("open fail");
        return NULL;
    }
    //2,获取bmp文件的尺寸信息！
    BITMAPINFODEADER info;
    bzero(&info,sizeof(info));
    char bmp_head[14]={0};
    fread(bmp_head,14,1,fn);//读掉bmp文件头效果刷图实现的基础本身就是要延时）
    fread(&info,sizeof(BITMAPINFODEADER),1,fn);//读取bmp信息头
#ifdef TEST
    printf("bmp_width:%d bmp_height:%d\n",info.biWidth,info.biHeight);
#endif
    //3,解码bmp文件，获取原始的RGB数据,处理颠倒问题！

    char *buf=calloc(1,info.biHeight*info.biWidth*3);
    if(buf==NULL)//（如果calloc分配内存出错，会返回NULL）
    {
        return buf;
    }
    char buf2[3];
    int num=0;
    while(1)
    {
        int ret=fread(buf,1,info.biHeight*info.biWidth*3,fn);
        num +=ret;
        if(ret == 0)
            break;
    }
#ifdef TEST
    printf("总读取：%d,  应读取：%d\n",num,info.biHeight*info.biWidth*3);
#endif
    for(int i=0;i<(int)info.biHeight/2;i++)
    {

        int offline=i*(int)info.biWidth*3;
        for(int j=0;j<(int)info.biWidth;j++)
        {
            buf2[0]=buf[j*3+offline];
            buf2[1]=buf[j*3+1+offline];
            buf2[2]=buf[j*3+2+offline];

            buf[j*3+offline]  =buf[j*3+(((int)info.biHeight-1)*(int)info.biWidth*3-offline)];
            buf[j*3+1+offline]=buf[j*3+1+(((int)info.biHeight-1)*(int)info.biWidth*3-offline)];
            buf[j*3+2+offline]=buf[j*3+2+(((int)info.biHeight-1)*(int)info.biWidth*3-offline)];

            buf[j*3+(((int)info.biHeight-1)*(int)info.biWidth*3-offline)]  =  buf2[0];
            buf[j*3+1+(((int)info.biHeight-1)*(int)info.biWidth*3-offline)]=  buf2[1];
            buf[j*3+2+(((int)info.biHeight-1)*(int)info.biWidth*3-offline)]=  buf2[2];

        }

    }
#ifdef TEST
    printf("bmp解码完毕！\n");
    printf("rgb_h:%d,rgb_w:%d\n",info.biHeight,info.biWidth);
#endif
    //以下通过pinfo找到链表所在的堆内存中的数据，并赋值！（区分栈内存和堆内存）
     pinfo->rgb_h=info.biHeight;
     pinfo->rgb_w=info.biWidth;
     pinfo->rgbBuf=buf;
     pinfo->flag=1;
     pinfo->decode_flag=1;
     buf_size++;
     fclose(fn);
     return buf;//这里依旧要返回值，是用来判断解码是否成功!
}

RGB _Jpg2Rgb(char *jpg_data,int   jpg_size)
{
    RGB rgb;
    /*用户自定义数据
    char *jpg_data = ...; // 需要解码的 JPG 数据
    int   jpg_size = ...; // 需要解码的 JPG 数据大小

    */
    /*
    char *rgb_data; // 解码后的 RGB 数据
    int   rgh_size; // 解码后的 RGB 数据大小
    */

    // 解码流程准备
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // 读取大小为 jpg_size 个字节的 jpg_data
    jpeg_mem_src(&cinfo, (const unsigned char *)jpg_data, jpg_size);

    // 读取JPEG文件的头，并判断其格式是否合法
    if(jpeg_read_header(&cinfo, true) != 1)
    {
        perror("读取JPG数据失败 ");
        exit(0);
    }

    // 开始解码 JPG 数据
    jpeg_start_decompress(&cinfo);

    rgb.rgb_width   = cinfo.output_width;
    rgb.rgb_height   = cinfo.output_height;
    int pix_size = cinfo.output_components; // 单位：字节

    // 根据图片的尺寸大小，分配一块相应的内存 rgb_data
    // 用来存放从 jpg_data 解码出来的图像数据
    unsigned long  rgb_size = rgb.rgb_width  * rgb.rgb_height * pix_size;
    rgb.rgb_data = calloc(1, rgb_size);

    // 循环地将图片的每一行读出并解码到 rgb_data 中
    int row_size = rgb.rgb_width * pix_size;
    while(cinfo.output_scanline < cinfo.output_height)
    {
        unsigned char *a[1];
        a[0] = (unsigned char *)(rgb.rgb_data) + (cinfo.output_scanline) * row_size;
        jpeg_read_scanlines(&cinfo, a, 1);
    }


    // 解码完了，释放相关资源
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    free(jpg_data);
    return rgb;


    // 至此，得到 RGB 数据和 RGB 数据的大小：
    // rgb_data
    // rgb_size

}
/*由于showJpg要用到GetJpg,所以GetJpg必须放到前面*/
char * GetJpg(FILE *fn,int Jpg_size)
{
   char *jpg_data=(char *)malloc(Jpg_size);
   fread(jpg_data,1,Jpg_size,fn);
   if(ferror(fn))
   {
       perror(("read fail"));
       exit(0);
   }

    return jpg_data;

}

char * Jpg2Rgb(linklist pinfo)
{
    //1,打开指定目录的JPG图片文件
    FILE *fn=fopen(pinfo->dname,"r");
    if(fn ==NULL)
    {

        perror("open fail");
        return NULL;
    }
    //2,获取JPG的文件大小
    struct stat info;
    //这里只需要文件名对应的字符串指针，不需要打开文件，原因是stat里面封装了相关打开文件的操作
    stat(pinfo->dname,&info);
    //3,获取JPG的原始数据
    char *jpg_data=GetJpg(fn,info.st_size);
    //4,解码JPG文件，获取相应的RGB文件
    RGB rgb=_Jpg2Rgb(jpg_data,info.st_size);


    /*
     由于通过测试，LCD的文件格式为BGRA，所以作一下调整：
    //RGB[0]  -> R
    //RGB[1]  -> G
    //RGB[2]  -> B

    //lcd[0]:  -> B
    //lcd[1]:  -> G
    //lcd[2]:  -> R
    //lcd[3]:  -> A

    通过上边的对应关系，在刷到LCD之前，RGB数据的R与B还要交换下顺序！
    */


    char buf=0x00;
    for(int i=0;i<rgb.rgb_width*rgb.rgb_height;i++)
    {
        buf=rgb.rgb_data[i*3];
        rgb.rgb_data[i*3]=rgb.rgb_data[i*3+2];
        rgb.rgb_data[i*3+2]=buf;
    }



    /*

        int redoffset   = vinfo.red.offset;//表示LCD的R在第(redoffset/8)个字节
        int greenoffset = vinfo.green.offset;//表示LCD的G在第(redoffset/8)个字节
        int blueoffset  = vinfo.blue.offset;//表示LCD的B在第(redoffset/8)个字节
     实际开发中，LCD的分辨率、RGB偏移量是直接靠上面的函数获取自动适应的，事先并不知道。
     所以加一下的判断来实现自适应RGB的偏移量（RGB数据的偏移量是已知的）

    */




#ifdef DEBUG
    printf("rgb_h:%d,rgb_w:%d\n",rgb.rgb_height,rgb.rgb_width);
#endif

    pinfo->rgb_h=rgb.rgb_height;
    pinfo->rgb_w=rgb.rgb_width;
    pinfo->rgbBuf=rgb.rgb_data;
    pinfo->flag=1;
    pinfo->decode_flag=1;
    buf_size++;
    fclose(fn);
    return rgb.rgb_data;


}



void MapAlbum_RGB(int type,
              char *p0,
              char *buf,
              int lcd_h,
              int lcd_w,
              int rgb_h,
              int rgb_w,
              int origin_x,
              int origin_y)
{
    int i=0,j=0;
    int jw=0;
    int jh=0;
    double k=0;
    double k_h;//纵向比例
    double k_w;//横向比例
    int final_k;
    char tpy=0x00;//透明度
    char black[3]={0xff,0xff,0xff};
    if(((rgb_w>lcd_w)||(rgb_h>lcd_h) )&&((type !=1)&&(type !=7)))
    {
        printf("暂未支持！\n");
        return;
    }
    switch(type)
     {


    case 0:
            for(i=0;i<lcd_h;i++)//刷某一行，一共有rgb.rgb_height行
            {
                long offset=i*lcd_w*4;	//LCD行的偏移量
                for(j=0;j<lcd_w;j++)
                {
                    memcpy(p0+4*j+offset,&black,3);
                    memcpy(p0+4*j+offset+3,&tpy,1);
                }
            }
            break;

    case 1:/*简单粗暴的刷图（无效果）*/
        if((rgb_w>lcd_w)||(rgb_h>lcd_h) )//jpg分辨率不适应
        {
            k=0;
            k_h=rgb_h/(double)lcd_h;//纵向比例
            k_w=rgb_w/(double)lcd_w;//横向比例
            /*
             * 根据LCD和RGB长宽比例得出某一合适的比例进行缩小！
             * 缩小原理：有规律地跳过RGB宽与与高的像素点进行局部的缩小以至于达到整体的缩小
             * 比如，宽高均缩小一半：RGB数据横向跳1刷1,纵向跳1刷1
             *
             */
            if(k_h-k_w>0.000001)
            {
                k=k_h;
            }
            else
            {
                k=k_w;
            }
            final_k=(int)k+1;//这里决定缩小程度。
            jw=0;
            jh=0;
            for(jh=0;jh<rgb_h;jh +=final_k)
            {

             /*
              *((lcd_h-(int)(rgb_h/(final_k)))/2))-->自动居中的纵向偏移量
              * ((lcd_w-(rgb_w/final_k))/2)       -->自动居中的横向偏移量
              * final_k                           -->缩小因子，跳过final_k字节，意味着图片宽或高相应缩小至（rgb_w/rgb_h）/final_k。
              * jw、jh                             -->决定RGB数据的像素点取值
              * i,j                               -->决定LCD数据像素点的取值
              */
                long offset=(i+((lcd_h-(int)(rgb_h/(final_k)))/2))*lcd_w*4;	//LCD行的偏移量
                long offset_rgb= (jh)*rgb_w*3;
                for(jw=0;jw<rgb_w;jw +=final_k)//一行有info.biWidth个像素点
                {
                    memcpy(p0+4*(j)+offset+((lcd_w-(rgb_w/final_k))/2)*4,buf+offset_rgb+(jw)*3,3);
                    memcpy(p0+4*(j)+offset+((lcd_w-(rgb_w/final_k))/2)*4+3,&tpy,1);

                    j++;

                }
                j=0;
                i++;

            }
            i=0;

        }

        else
        {
            for(i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
            {
                long offset=(i+((lcd_h-(int)rgb_h)/2))*lcd_w*4;	//LCD行的偏移量
                for(j=0;j<(int)rgb_w;j++)//一行有info.biWidth个像素点
                {
                    /*rgb_w*3*i为RGB的行偏移量*/
                    memcpy(p0+4*j+offset+((lcd_w-(int)rgb_w)/2)*4,buf+((int)rgb_w*3*i+j*3),3);
                    memcpy(p0+4*j+offset+((lcd_w-(int)rgb_w)/2)*4+3,&tpy,1);
                }
            }
        }

      break;
    case 2:/*从上向下刷的延时效果*/
         for(i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
         {
             long offset=(i+((lcd_h-(int)rgb_h)/2))*lcd_w*4;	//LCD行的偏移量
             for(j=0;j<(int)rgb_w;j++)//一行有info.width个像素点
             {
                 /*rgb_w*3*i为RGB的行偏移量*/
                 memcpy(p0+4*j+offset+((lcd_w-(int)rgb_w)/2)*4,buf+((int)rgb_w*3*i+j*3),3);
                 memcpy(p0+4*j+offset+((lcd_w-(int)rgb_w)/2)*4+3,&tpy,1);
             }
             usleep(50);
         }
         break;
    case 3:/*从下往上刷的延时效果*/
         for(i=(int)rgb_h-1;i>=0;i--)//刷某一行，一共有rgb.rgb_height行
         {
             //long offset=(i-((lcd_h-(int)rgb_h)/2))*lcd_w*4;
             //|
             //-->错误写法，实践发现，该错误写法在刷bmp时仍然能够成功刷出图片，而在刷jpg时出现了段错误！
             //这个错误隐晦点在于刷bmp是能够成功刷出图片的，导致误认为其他地方出现了问题！
             //其实取临界条件分析，取i=0,显而易见 offset小于零，这绝对错误的。
             //这里的问题可得知经过JPG库解码后的相同大小的RGB数据与bmp解码后的RGB数据相比，尽管他们有一些差别。
             //但是在图片上看是基本一致的！问题的怪异点在于bmp对应的RGB原始数据经过映射加工后居然没有段错误！
             //至少从这个方面表明，即使内存大小一致RGB数据，段错误却与其没有关联。
             //因为这里的段错误极大可能是越界访问了其他内存，刷bmp时没有出现问题表明此时只是运气好碰到了越界的访问的内存没有被占用！
             //总结：在越界的情况下，段错误也不是绝对出现的！可能存在，也可能不存在！
             //但是值得肯定的是，在发生段错误时，必然是由内存越界访问、或是一些野指针导致等原因的！
             //所以在发生段错误时，首先判断的是自己的代码一定写得有些问题，即使在某些局部效果上看没有问题（如这个在bmp成功刷出！）
              long offset=(lcd_h-(lcd_h-(int)rgb_h)/2+(i-(int)rgb_h+1))*lcd_w*4;	//LCD行的偏移量
             for(j=0;j<(int)rgb_w;j++)//一行有info.width个像素点
             {
                 /*rgb_w*3*i为RGB的行偏移量*/
                 memcpy(p0+4*j+offset+((lcd_w-(int)rgb_w)/2)*4,buf+((int)rgb_w*3*i+j*3),3);
                 memcpy(p0+4*j+offset+((lcd_w-(int)rgb_w)/2)*4+3,&tpy,1);
             }
             usleep(50);
         }
         break;
    case 4:/*从左往右刷的延时效果*/
         for(i=0;i<(int)rgb_w;i++)
         {

             long offsetL=4*(i+((lcd_w-(int)rgb_w)/2));
             long offsetR=3*(i);
             for(j=0;j<(int)rgb_h;j++)
             {

                 memcpy(p0+4*lcd_w*(j+((lcd_h-(int)rgb_h)/2))+offsetL,buf+offsetR+3*(int)rgb_w*j,3);
                 memcpy(p0+4*lcd_w*(j+((lcd_h-(int)rgb_h)/2))+offsetL+3,&tpy,1);
             }
             usleep(50);
         }
         break;

    case 5:/*从右往左刷的延时效果*/
         for(i=(int)rgb_w-1;i>=0;i--)
         {
             long offsetL=4*(lcd_w-((lcd_w-(int)rgb_w)/2)+(i+1-(int)rgb_w));
             long offsetR=3*(i);
             for(j=0;j<(int)rgb_h;j++)
             {

                 memcpy(p0+4*lcd_w*(j+((lcd_h-(int)rgb_h)/2))+offsetL,buf+offsetR+3*(int)rgb_w*j,3);
                 memcpy(p0+4*lcd_w*(j+((lcd_h-(int)rgb_h)/2))+offsetL+3,&tpy,1);
             }
             usleep(50);
         }
         break;


    case 6:/*上下方向的百叶窗效果*/
              for(i=0;i<(int)rgb_h/4;i++)
              {
                  long offsetL=(i+((lcd_h-(int)rgb_h)/2))*lcd_w*4;	              //LCD行的偏移量
                  long offsetR=((int)rgb_w*3*i);     //RGB的行偏移量
                  for(int k=0;k<4;k++)
                  {
                      offsetL=lcd_w*4*(i+((int)rgb_h/4)*k+((lcd_h-(int)rgb_h)/2));
                      offsetR=(int)rgb_w*3*(i+((int)rgb_h/4)*k);
                      for(j=0;j<(int)rgb_w;j++)//一行有info.width个像素点
                      {

                          memcpy(p0+4*(j+((lcd_w-(int)rgb_w)/2))+offsetL,buf+offsetR+3*j,3);
                          memcpy(p0+4*(j+((lcd_w-(int)rgb_w)/2))+offsetL+3,&tpy,1);
                      }

                  }
                  usleep(100);
              }
              break;

    case 7:/*左右方向的百叶窗效果*/

        if((rgb_w>lcd_w)||(rgb_h>lcd_h) )//jpg分辨率不适应
        {
            k=0;
            k_h=rgb_h/(double)lcd_h;//纵向比例
            k_w=rgb_w/(double)lcd_w;//横向比例
            //根据LCD和RGB长宽比例得出某一合适的比例进行缩小！
            if(k_h-k_w>0.000001)
            {
                k=k_h;
            }
            else
            {
                k=k_w;
            }
            final_k=(int)k+1;//这里决定缩小程度。

            jw=0;
            jh=0;
            for(jw=0;jw<rgb_w/4;jw +=final_k)//刷某一行，一共有rgb.rgb_height行
            {

                for(int k=0;k<4;k++)
                {


                    long offsetL=4*(i+((int)(rgb_w/(final_k))/4)*k+((lcd_w-(int)(rgb_w/final_k))/2));
                    long offsetR=3*((jw)+((int)rgb_w/4)*k);

                    for(jh=0;jh<rgb_h;jh +=final_k)//一行有info.biWidth个像素点
                    {

                        memcpy(p0+4*lcd_w*(j+((lcd_h-(int)(rgb_h/(final_k)))/2))+offsetL,buf+offsetR+3*(int)rgb_w*(jh),3);
                        memcpy(p0+4*lcd_w*(j+((lcd_h-(int)(rgb_h)/(final_k))/2))+offsetL+3,&tpy,1);

                        j++;

                    }
                    /*这里一开始将i++写到K循环体内这里导致无法正常实现百叶窗效果。
                     * 理由是将i++写在k循环内，会造成LCD在横向偏移时造成错误的累加偏移！
                     * 这里犯错误是没有跟踪到每一处循环对应的刷图细节，仔细琢磨便能得出错误点！
                    */
                    j=0;
                }
                i++;
                usleep(100);

            }
            printf("act_w:%d\n",i);
            i=0;
            break;

        }
             for(i=0;i<(int)rgb_w/4;i++)
             {

                 for(int k=0;k<4;k++)
                 {
                     long offsetL=4*(i+((int)rgb_w/4)*k+((lcd_w-(int)rgb_w)/2));
                     long offsetR=3*(i+((int)rgb_w/4)*k);
                     for(j=0;j<(int)rgb_h;j++)
                     {

                         memcpy(p0+4*lcd_w*(j+((lcd_h-(int)rgb_h)/2))+offsetL,buf+offsetR+3*(int)rgb_w*j,3);
                         memcpy(p0+4*lcd_w*(j+((lcd_h-(int)rgb_h)/2))+offsetL+3,&tpy,1);
                     }
                 }
                 usleep(100);
             }
             break;

        case 8:/*刷预览图！*/



            {
                for(i=0;i<(int)rgb_h;i++)//刷某一行，一共有rgb.rgb_height行
                {
                    long offset=(i+origin_y)*lcd_w*4;	//LCD行的偏移量
                    for(j=0;j<(int)rgb_w;j++)//一行有info.biWidth个像素点
                    {
                        /*rgb_w*3*i为RGB的行偏移量*/
                        memcpy(p0+4*j+offset+(origin_x)*4,buf+((int)rgb_w*3*i+j*3),3);
                        memcpy(p0+4*j+offset+(origin_x)*4+3,&tpy,1);
                    }
                }
            }

          break;
     }

}



/*在LCD上刷一张RGB数据的图片，失败返回NULL*/
char * showRgb(linklist pinfo,int type,int origin_x,int origin_y )
{
    //这里存储一个函数内部的静态变量，来记录进入函数的次数（初始化只会执行一次！）

         static int i_flag=0;


    //1,判断图片是否已经解码并存在缓冲区，若无则解码，若有则跳过！
    if(pinfo != NULL)
    {
        if(pinfo->flag ==0 )
        {
            if(strstr(pinfo->format,"jpg") != NULL)
            {
                Jpg2Rgb(pinfo);
                if(pinfo->rgbBuf==NULL)
                    return NULL;
            }
            else if(strstr(pinfo->format,"bmp") != NULL)
            {
                Bmp2Rgb(pinfo);
                if(pinfo->rgbBuf==NULL)
                    return NULL;

            }
            else
            {
                printf("不支持的图片格式\n");
                return NULL;
            }

        }

    }

    int rgb_h;
    int rgb_w;
    //2,打开LCD文件
    if(pinfo != NULL)
    {
        rgb_h=pinfo->rgb_h;
        rgb_w=pinfo->rgb_w;
    }
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

    if(pinfo != NULL)
    {
        MapAlbum_RGB(type,p0,pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);
    }
    if(pinfo == NULL)
    {
        MapAlbum_RGB(0,p0,NULL,h,w,h,w,origin_x,origin_y);
    }


    /*

    //实现闪屏优化，只针对普通刷图的闪屏优化，效果刷图是不需要优化的！（效果刷图实现的基础本身就是要延时）
    if(type==1 || type==0 || type==8)
    {
        printf("行号：%d\n",__LINE__);
        //如果是第一次进入该函数
        if(i_flag==0)
        {
            MapAlbum_RGB(0,p0,NULL,h,w,h,w,origin_x,origin_y);

            vinfo.xoffset=0;
            vinfo.yoffset=0;
            if(ioctl(fd,FB_ACTIVATE_NOW,&vinfo))//使其立即生效
            {
               perror("ioctl()");
            }


           ioctl(fd,FBIOPAN_DISPLAY,&vinfo);//配置属性扫描显示
           if(pinfo != NULL)
           {
               MapAlbum_RGB(type,p0+(h*w*(bpp/8)),pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);
           }
           if(pinfo == NULL)
           {
               MapAlbum_RGB(0,p0+(h*w*(bpp/8)),NULL,h,w,h,w,origin_x,origin_y);
           }





           vinfo.xoffset=0;
           vinfo.yoffset=480;
           if(ioctl(fd,FB_ACTIVATE_NOW,&vinfo))//使其立即生效
           {
              perror("ioctl()");
           }

           ioctl(fd,FBIOPAN_DISPLAY,&vinfo);//配置属性扫描显示
           i_flag++;

        }
        //不是第一次，且是奇数次进入该函数！这样做的目的是实现双倍缓冲区的交替刷图
        else if(i_flag !=0 &&(i_flag)%2 != 0)
        {
            MapAlbum_RGB(0,p0,NULL,h,w,h,w,origin_x,origin_y);
            if(pinfo != NULL)
            {
                MapAlbum_RGB(type,p0,pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);
            }
            if(pinfo == NULL)
            {
                MapAlbum_RGB(0,p0,NULL,h,w,h,w,origin_x,origin_y);
            }

            vinfo.xoffset=0;
            vinfo.yoffset=0;
            if(ioctl(fd,FB_ACTIVATE_NOW,&vinfo))//使其立即生效
            {
               perror("ioctl()");
            }

            ioctl(fd,FBIOPAN_DISPLAY,&vinfo);//配置属性扫描显示
            i_flag++;

        }
        //不是第一次，且是偶数次进入该函数
        else if(i_flag !=0 &&(i_flag)%2 == 0)
        {
            MapAlbum_RGB(0,p0+(h*w*(bpp/8)),NULL,h,w,h,w,origin_x,origin_y);
            if(pinfo != NULL)
            {
                MapAlbum_RGB(type,p0+(h*w*(bpp/8)),pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);
            }
            if(pinfo == NULL)
            {
                MapAlbum_RGB(0,p0+(h*w*(bpp/8)),NULL,h,w,h,w,origin_x,origin_y);
            }
            vinfo.xoffset=0;
            vinfo.yoffset=480;
            if(ioctl(fd,FB_ACTIVATE_NOW,&vinfo))//使其立即生效
            {
               perror("ioctl()");
            }

            ioctl(fd,FBIOPAN_DISPLAY,&vinfo);//配置属性扫描显示
            i_flag=1;

        }

    }

    //若是要实现百叶窗效果，由于引进了可见区的概念，必须判断当前可见区在那儿！

    if(type !=1 &&type !=0 &&type !=8&&i_flag ==0)
    {
        MapAlbum_RGB(0,p0,NULL,h,w,h,w,origin_x,origin_y);
        MapAlbum_RGB(type,p0,pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);

    }

    else if(type !=1 &&type !=0 &&type !=8 &&i_flag !=0&&(i_flag%2) ==0)
    {
        MapAlbum_RGB(0,p0,NULL,h,w,h,w,origin_x,origin_y);
        MapAlbum_RGB(type,p0,pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);

    }

    else if(type !=1 &&type !=0 &&type !=8 &&i_flag !=0&&(i_flag%2) !=0)
    {
        MapAlbum_RGB(0,p0+(h*w*(bpp/8)),NULL,h,w,h,w,origin_x,origin_y);
        MapAlbum_RGB(type,p0+(h*w*(bpp/8)),pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);

    }

    */

     //5,释放映射内存等的资源
     munmap(p0,w*h*(bpp/8));
     close(fd);
     if(pinfo != NULL)
     {
          return pinfo->rgbBuf;
     }
     return NULL;

}




/*
  支持bmp、jpg图片格式，实现了多种刷图效果，增设缓冲区机制！(存储上五次不重复的浏览的图片的RGB数据！)
*/
 char * showPicture(linklist p,int type)
 {
    //showBlack();//刷其他图片之前先清屏

     showRgb(p,type,0,0);
     return NULL;

 }


 //刷缩略图
 char * showPreview(linklist p,int origin_x, int origin_y)
 {
     if(p ==NULL)
     {
         return NULL;
     }
    //showBlack();//刷其他图片之前先清屏！
     if(buf_size ==5)//缓冲区满了，清空所有缓冲区！
     {
         linklist tmp;
         struct list_head *pos;//指向小结构体的指针
         struct list_head *n;

         list_for_each_safe(pos,n,&(p->list))//从当前节点的下一个节点出发遍历
         {
             tmp=list_entry(pos,struct node,list);
             if(tmp->flag ==1)//如果存在缓冲区
             {
                 /*错误：free(p->rgbBuf);
                  * 这里应是当前tmp下的指向的图片缓冲区释放，这个段错误犯的不应该，找了好久。。。
                  * 重复free导致的段错误(类型：p->rgbBuf第二次使指向非法空间！即释放了，再次指向使用！)
                  * 经典的  double free！！

                   */
                 free(tmp->rgbBuf);
                 tmp->flag=0;//表示不存在
                 buf_size--;
             }

         }
         if(p->flag==1)//如果该节点存在缓冲区，把这个节点的图片缓冲区释放
         {
             free(p->rgbBuf);
             p->flag=0;//表示不存在
             buf_size--;
         }

     }

     showRgb(p,8,origin_x,origin_y);
     return NULL;

 }



 //进度条大小为800*130 JPG
 char * showBar(linklist pinfo,int origin_x,int origin_y,int flag)
 {

     //解码jpg，获取大小
     Jpg2Rgb(pinfo);
     int rgb_h;
     int rgb_w;
     //2,打开LCD文件
     if(pinfo != NULL)
     {
         rgb_h=pinfo->rgb_h;
         rgb_w=pinfo->rgb_w;
     }
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


     MapAlbum_RGB(flag,p0,pinfo->rgbBuf,h,w,rgb_h,rgb_w,origin_x,origin_y);


     return NULL;


 }

