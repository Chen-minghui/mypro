
#include "myplayer.h"

extern int size;//引用主函数的全局变量
linklist init_list(void)
{
    linklist head=calloc(1,sizeof(linknode));
    if(head ==NULL)
        return NULL;
     bzero(head,sizeof(linknode));
     head->flag=0;
    INIT_LIST_HEAD(&head->list);
    return head;
}

linklist new_node(char *dname,char *vname,int num_preview)
{
    linklist p=calloc(1,sizeof(linknode));
    char *format=NULL;
    if(p ==NULL)
        return NULL;
    INIT_LIST_HEAD(&p->list);
    strcpy(p->dname,dname);
    strcpy(p->vname,vname);
    p->num_preview=num_preview;
    p->flag=0;
    format=CmpFormat(dname);
    printf("format格式：%s\n",format);
    if(strcmp(format,"jpg")==0||strcmp(format,"bmp")==0)
    {

        strcpy(p->format,format);
       printf("p->format格式：%s\n",p->format);

    }
    size++;
    return p;
}
linklist Add_list(linklist head,char *dname,int num_preview)
{
    linklist p=calloc(1,sizeof(linknode));
    if(p ==NULL)
        return NULL;
    INIT_LIST_HEAD(&p->list);
    strcpy(p->dname,dname);
    p->num_preview=num_preview;
    p->flag=0;
    size++;
    list_add_tail(&(p->list),&(head->list));
    return p;
}

void travel_list(linklist head,void (*func)(linklist ))
{
    struct list_head *pos;//指向小结构体的指针
    struct list_head *n;
    list_for_each_safe(pos,n,&(head->list))
    {
        linklist p=list_entry(pos,struct node,list);
        func(p);
    }
    printf("共计%d张图片\n",size);
}

struct list_head * Find_name(linklist head,char *path)
{
    struct list_head *pos;//指向小结构体的指针
    struct list_head *n;
    list_for_each_safe(pos,n,&(head->list))
    {

        linklist p=list_entry(pos,struct node,list);
        if(strstr(p->vname,path) != NULL )
        {
            return pos;
        }
    }
   return NULL;
}

linklist Find_dname(linklist head,char *path)
{
    if(path ==NULL)
    {
        return NULL;
    }
    struct list_head *pos;//指向小结构体的指针
    struct list_head *n;
    list_for_each_safe(pos,n,&(head->list))
    {

        linklist p=list_entry(pos,struct node,list);
        if(strstr(p->dname,path) != NULL )
        {
            return p;
        }
    }
   return NULL;
}


void travel_PresentNode(linklist p,void (*func)(char *,char *, char *,int ))
{
    printf("当前节点的数据信息如下：\n");
    struct list_head *pos=&p->list;//指向小结构体的指针
    p=list_entry(pos,struct node,list);
    func(p->vname,p->dname,p->format,p->num_preview);
    printf("\n");
}

void list_destroy(linklist *head)
{
    struct list_head *pos;//指向小结构体的指针
    struct list_head *n;
    list_for_each_safe(pos,n,&((*head)->list))
    {
        linklist p=list_entry(pos,struct node,list);
        bzero(p,sizeof(linknode));
        free(p->rgbBuf);
        free(p);
    }

}
