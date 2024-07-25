#include "socket.h"


int g_socketfd = 0;
Node *head;

extern fd_set READSET;
extern int g_maxfd;
extern int g_start_flag;
extern int g_suspend_flag;

void send_server(int sig)
{
	struct json_object *SendObj = json_object_new_object();
	json_object_object_add(SendObj,"cmd",json_object_new_string("info"));

	Shm s;
	memset(&s,0,sizeof(s));
	get_shm(&s);


	json_object_object_add(SendObj,"cur_music",json_object_new_string(s.cur_music));	
	
	int volume;
	get_volume(&volume);
	json_object_object_add(SendObj,"volume",json_object_new_int(volume));

	json_object_object_add(SendObj,"mode",json_object_new_int(s.mode));

	json_object_object_add(SendObj,"deviceid",json_object_new_string("0001"));
	
	if(g_start_flag == 0)
	{
		json_object_object_add(SendObj,"status",json_object_new_string("stop"));
	}
	else if(g_start_flag == 1 && g_suspend_flag == 1)
	{
		json_object_object_add(SendObj,"status",json_object_new_string("suspend"));
	}
	else if(g_start_flag == 1 && g_suspend_flag == 0)
	{
		json_object_object_add(SendObj,"status",json_object_new_string("start"));
	}

	socket_send_data(SendObj);

	json_object_put(SendObj);

	alarm(2);
}

void socket_send_data(struct json_object *obj)
{
	char msg[1024] = {0};
	const char *s = json_object_to_json_string(obj);
	int len = strlen(s);

	memcpy(msg,&len,sizeof(int));
	memcpy(msg + sizeof(int),s,len);

	if(send(g_socketfd,msg,len + 4, 0) == -1)
	{
		perror("send");
	}




}

void socket_recv_data(char *msg)
{
	int len;
	ssize_t size = 0;
	char buf[1024] = {0};

	while(1)
	{
		size += recv(g_socketfd,buf + size,sizeof(int) - size,0);
		if(size >= sizeof(int))
		{
			break;
		}

		
	}
	
	len = *(int *)buf;

	printf("---LENGTH: %d",len);
	size = 0;
	memset(buf,0,1024);
	
	while(1)
	{

		size += recv(g_socketfd,msg + size,len - size,0);
		if(size >= len)
		{
			break;
		}
	}

	printf("MSG: %s\n",msg);
}

int init_socket()
{
	int count = 50;
	
	g_socketfd = socket(PF_INET,SOCK_STREAM,0);
	if(g_socketfd == -1)
	{
		perror("socket");
		return -1;
	}
	
	struct sockaddr_in server_info;
	memset(&server_info,0,sizeof(server_info));
	server_info.sin_family = PF_INET;
	server_info.sin_port = htons(PORT);
	server_info.sin_addr.s_addr = inet_addr(IP);
	
	int len = sizeof(server_info);
	
	while(count--)
	{
		if(connect(g_socketfd,(struct sockaddr *)&server_info,len) == -1)
		{
			printf("CONNECT SERVER FAILURE ......\n");
			sleep(1);
			continue;
		}
#ifdef ARM
		start_buzzer();
#endif
				
		FD_SET(g_socketfd,&READSET);

		g_maxfd = ( g_maxfd < g_socketfd) ? g_socketfd : g_maxfd;
		
		//alarm(2);
		signal(SIGALRM,send_server);

		send_server(SIGALRM);

		break;

	}

}

void upload_music_list()
{
	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd", json_object_new_string("upload_music"));

	struct json_object *arr = json_object_new_array();
	Node *p = head->next;
	while(p)
	{
		json_object_array_add(arr,json_object_new_string(p->music_name));
		p = p->next;
	}	
	json_object_object_add(obj,"music",arr);
	socket_send_data(obj);

	json_object_put(obj);
	json_object_put(arr);

}

void socket_start_play()
{
	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj, "cmd",json_object_new_string("app_start_reply"));

	if( -1 == start_play())
	{
		json_object_object_add(obj,"result",json_object_new_string("failure"));
	}
	else
	{
		json_object_object_add(obj,"result",json_object_new_string("success"));
	}

	socket_send_data(obj);
	json_object_put(obj);
}

void socket_stop_play()
{
	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_stop_reply"));
	stop_play();
	
	json_object_object_add(obj,"result",json_object_new_string("success"));
	
	socket_send_data(obj);

	json_object_put(obj);

}


void socket_suspend_play()
{
	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_suspend_reply"));

	suspend_play();
	json_object_object_add(obj,"result",json_object_new_string("success"));

	socket_send_data(obj);

	json_object_put(obj);

}


void socket_continue_play()
{

	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_continue_reply"));

	continue_play();
	json_object_object_add(obj,"result",json_object_new_string("success"));

	socket_send_data(obj);

	json_object_put(obj);


}

void socket_next_play()
{
	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_next_reply"));

	next_play();

	json_object_object_add(obj,"result",json_object_new_string("success"));

	Shm s;
	get_shm(&s);

	json_object_object_add(obj,"music",json_object_new_string(s.cur_music));
	
	socket_send_data(obj);
	json_object_put(obj);


}


void socket_prior_play()
{

	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_prior_reply"));

	prior_play();

	json_object_object_add(obj,"result",json_object_new_string("success"));

	Shm s;
	get_shm(&s);

	json_object_object_add(obj,"music",json_object_new_string(s.cur_music));
	
	socket_send_data(obj);
	json_object_put(obj);


}


void socket_voice_up()
{
	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_voice_up_reply"));
	voice_up();

	json_object_object_add(obj,"result",json_object_new_string("success"));

	int volume;
	get_volume(&volume);

	json_object_object_add(obj,"voice",json_object_new_int(volume));

	socket_send_data(obj);

	json_object_put(obj);

}

void socket_voice_down()
{

	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_voice_down_reply"));
	voice_down();

	json_object_object_add(obj,"result",json_object_new_string("success"));

	int volume;
	get_volume(&volume);

	json_object_object_add(obj,"voice",json_object_new_int(volume));

	socket_send_data(obj);

	json_object_put(obj);



}

void socket_circle_play()
{

	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_circle_reply"));
	circle_play();	

	json_object_object_add(obj,"result",json_object_new_string("success"));

	socket_send_data(obj);

	json_object_put(obj);


}

void socket_sequence_play()
{

	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("app_sequence_reply"));
	sequence_play();	

	json_object_object_add(obj,"result",json_object_new_string("success"));

	socket_send_data(obj);

	json_object_put(obj);

}
