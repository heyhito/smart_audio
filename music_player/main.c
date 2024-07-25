#include "main.h"

fd_set READSET;
int g_maxfd;
Node *head = NULL;

int main()
{
	system("./init.sh");
	//初始化select
	init_select();

	//初始化设备文件
	if(init_device() == -1)
	{
		printf("初设备文件失败\n");
	}
	//exit1(1);



	//初始化网络
	if(init_socket() == -1)
	{
		printf("初始化网络失败\n");
		return -1;
	}
	printf("网络连接成功\n");
	
	//初始化共享内容
	if(init_shm() == -1)
	{
		printf("初始化共享内存失败\n");
		return -1;
	}
	
	//初始化链表
	if(init_link() == -1)
	{
		printf("初始化链表失败\n");
		return -1;
	}

	
	//获取音乐
	get_music("其他");	
	//traverse_link();
	
	signal(SIGUSR1,update_music);

	//监听设备
	m_select();

	return 0;
}

