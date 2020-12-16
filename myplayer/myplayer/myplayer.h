#ifndef MYPLAYER_H
#define MYPLAYER_H
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/fb.h>//存放相关设备的说明书
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include"kernellist.h"
#include"jpeglib.h"
#include"thread_pool.h"

/*采用内核链表作为容器存储图片/视频文件名*/
typedef struct node{

    char dname[100];//图片的名字
    char vname[100];//视频文件的名字
    char format[10];//图片的格式类型
    int rgb_h;//图片的高
    int rgb_w;//图片的宽
    char *rgbBuf;//指向图片的RGB数据缓冲区
    int flag;//表明图片是否解码存在缓冲区标志位
    int decode_flag;//表明图片曾经是否被解码过的标志位
    int num_preview;//预览图编号，决定放在LCD的哪一块，分别为 1 2 3 4,    (0表示普通图片！)
    struct list_head list;
}linknode,*linklist;



/*

该结构体为GetPoint函数中返回值的声明定义，功能如下：

1）单击行为的满足条件： flag<=2*n  (n为区间误差)

2）滑动行为满足条件：   比如dx=1&&dy=0表示水平向右滑动！


*/
typedef struct Point{//存储坐标信息

    int X;//松手前最新一次的X坐标
    int Y;//松手前最新一次的Y坐标
    int dx;//判断松手前坐标X轴方向的移动（终点X-起点X）
    int sum_x;//统计按下到松手时x轴移动的距离
    int sum_y;//统计按下到松手时y轴移动的距离
    int dy;//判断松手前坐标Y轴方向的移动（终点Y-起点Y）
    int flag;//存储松手前移动了的坐标数量信息

}Point;



/*bmp的头数据包括文件头和信息头，分别占14字节、40字节，以下是信息头的结构体，用来获取长和宽*/
typedef struct BITMAPINFODEADER
{
    u_int32_t biSize;
    u_int32_t biWidth;
    u_int32_t biHeight;
    u_int16_t biPlanes;
    u_int16_t biBitCount;
    u_int32_t biCompression;
    u_int32_t biSizeImage;
    u_int32_t biXPelsPerMeter;
    u_int32_t biYPelsPerMeter;
    u_int32_t biClrUsed;
    u_int32_t biClrImportant;
}BITMAPINFODEADER;


typedef struct
{
    char * rgb_data;
    int rgb_width;
    int rgb_height;
}RGB;


int cpyInfo (char *org_path,char *cpy_path);
int Get_Vpic(linklist head);
int PreView(linklist head);
void Interface(linklist head);

linklist init_list(void);
linklist new_node(char *dname,char *vname,int num_preview);
linklist Add_list(linklist head,char *dname,int num_preview);
struct list_head * Find_name(linklist head,char *path);
linklist Find_dname(linklist head,char *path);
void travel_list(linklist head,void (*func)(linklist ));
void travel_PresentNode(linklist p,void (*func)(char *,char *, char *,int ));
void list_destroy(linklist *head);

/*处理图片数据的辅助函数*/
char *  CmpFormat(char *dname);
Point GetPoint(void);
char * Bmp2Rgb(linklist pinfo);
RGB _Jpg2Rgb(char *jpg_data,int   jpg_size);
char * Jpg2Rgb(linklist pinfo);

void MapAlbum_RGB(int type,
              char *p0,
              char *buf,
              int lcd_h,
              int lcd_w,
              int rgb_h,
              int rgb_w,
              int origin_x,
              int origin_y);
char * showRgb(linklist pinfo,int type,int origin_x,int origin_y );
 char * showBar(linklist pinfo,int origin_x,int origin_y,int flag);

/*以下为实际使用的刷图函数*/
char * showPicture(linklist p,int type);
 char * showPreview(linklist p,int origin_x, int origin_y);
void SerachAlbum(linklist head,char *path);


void *task(void *arg);


#endif // MYPLAYER_H
