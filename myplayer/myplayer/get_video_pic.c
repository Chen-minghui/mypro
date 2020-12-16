#include "myplayer.h"


extern int size;//引用主函数的全局变量

int cpyInfo (char *org_path,char *cpy_path)
{
   // char org_path[100]={0};//源文件绝对路径
   // char cpy_path[100]={0};//新文件绝对路径

    //拷贝文件
    FILE *fp1 = fopen(org_path, "r"); // 只读打开，并要求文件必须存在
    FILE *fp2 = fopen(cpy_path, "w"); // 只写打开，并在文件已有的情况下清空
    if(fp1 == NULL || fp2 == NULL)
    {
        perror("fopen失败");
        exit(0);
    }

    // 2，不断地从fp1读取数据，放入fp2
    char *buf = malloc(1024); // 每次读取1K的数据
    while(1)
    {
        bzero(buf, 1024);
        long pos1 = ftell(fp1);
        int n = fread(buf, 4, 1024/4, fp1);

        // 正常读取到1k字节
        if(n == 1024/4)
        {
            // 将1k数据写入fp2中
            fwrite(buf, 4, 1024/4, fp2);
        }

        else if(n < 1024/4)
        {
            // 1，遇到了错误，玩不下去了
            if(ferror(fp1))
            {
                perror("读取文件失败");

                //errno是在errno.h声明的全局变量，该错误码是由于某种信号意外终止。
                //所以修复这个错误十分简单，终止当次循环，进行下一次循环重新读取数据！
                if(errno == EINTR)//如果错误码为EINTR
                {
                    continue;

                }
            }

            // 2，遇到了不足2块的数据，即到达了文件末尾
            if(feof(fp1))
            {
                long pos2 = ftell(fp1);
                fwrite(buf, pos2-pos1, 1, fp2);
                break;
            }
        }
    }

    // 3，关闭文件，释放资源
    fclose(fp1);
    fclose(fp2);
    free(buf);
    return 0;

}




//获取视频的预览图，大小均为250*150  JPG格式
int Get_Vpic(linklist head)
{

    //1,创建一个截图的缓冲目录
    mkdir("/root/myplayer/buf_pic",0777);
   //创建一个目录来装视频的预览图
     mkdir("/root/myplayer/video_pic",0777);
   chdir("/root/myplayer/buf_pic");//主线程进到该目录,让mplayer将截图放在该目录
   // FILE* mp;
    char buf[1000];
    char buf_name[1000]={0};
    int num=1;
    //float timepos;
    char p='1';


    //2,遍历video目录,一个个地打开视频并截取该视频的某一帧作为截图！
    DIR *fp=opendir("/root/myplayer/video");
    if(fp == NULL)
    {
        perror("open fail!");

    }
    while(1)
    {
        struct dirent *ep=readdir(fp);
        if(ep ==NULL)
        {
            break;
        }
        if(ep->d_name[0] =='.')//不显示隐藏文件
        {
            continue;
        }

        if((strstr(ep->d_name,"mp4") != NULL) || (strstr(ep->d_name,"avi") != NULL))//遇到特定格式的视频才播放！
        {
            /*

            //打开某个视频文件
            sprintf(buf_name,"mplayer -quiet -slave -input file=/root/pipe -zoom -x 250 -y 150  '/root/myplayer/video/%s'",ep->d_name);
            mp=popen(buf_name,"r");
            bzero(buf_name,1000);

            //播放一帧，然后暂停
            system("echo \"frame_step\" > /root/pipe");
            fgets(buf, 1000, mp);
            printf("%s\n",buf);
            bzero(buf,1000);
            sleep(1);

            */

            //截图！
            sprintf(buf,"/root/myplayer/video_pic/pre_%c.jpg",p++);
            chdir("/root/myplayer/buf_pic");//主线程进到该目录,让mplayer将截图放在该目录
  sprintf(buf_name,"mplayer -ss 1 -noframedrop -nosound -vo jpeg  -frames 1 -zoom -x 250 -y 150 /root/myplayer/video/%s",ep->d_name);
            system(buf_name);
            if(access("/root/myplayer/buf_pic/00000001.jpg",F_OK) ==0)
            {
                printf("截图成功！\n");
            }
            cpyInfo("/root/myplayer/buf_pic/00000001.jpg",buf);
            bzero(buf_name,1000);

            /*
            system("killall -9 mplayer");

            sprintf(buf_name,"/root/myplayer/video_pic/%s",p);
            rename("/root/myplayer/video_pic/00000001.jpg", buf_name);
            p++;
            bzero(buf_name,1000);
            */

            //将预览图、视频名加入到链表中的数据节点，形成一一对应的关系！
            char buf_vname[100]={0};
            sprintf(buf_vname,"/root/myplayer/video/%s",ep->d_name);
            linklist p=new_node(buf,buf_vname,num);
            list_add_tail(&p->list,&head->list);

            bzero(buf,1000);
            /*
            if(remove("/root/myplayer/buf_pic/00000001.jpg") == -1)
            {
                perror("删除失败");
            }
            */

            num++;
            if(num ==5)
            {
                num=1;
            }


        }
    }
    printf("视频预览图获取完毕！\n");
    return 0;


}
