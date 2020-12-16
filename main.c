#include<stdio.h>

int main()
{
	char buf[1024]={0};
	printf("请输入一个字符串\n");
	scanf("%s",buf);
	printf("Hello world!\n");
	printf("输入的字符串为：%s\n",buf);
	return 0;

}
