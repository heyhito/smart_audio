#include "player.h"

int g_shmid = 0;
int g_start_flag = 0;
int g_suspend_flag = 0;
extern Node *head;
extern int g_mixerfd;

int init_shm()
{
	g_shmid = shmget(SHMKEY, SHMSIZE, IPC_CREAT | IPC_EXCL | 0666);
	if(-1 == g_shmid)
	{
		perror("shmget");
		return -1;
	}

	void *addr = shmat(g_shmid,NULL,0);
	if((void *)-1 == addr)
	{
		perror("open shmat");
		return -1;
	}
	
	Shm s;
	memset(&s,0,sizeof(s));
	
	s.parent_pid = getpid();

	s.mode = SEQUENCE;
	memcpy(addr,&s,sizeof(s));
	
	shmdt(addr);

}

void get_shm(Shm *s)
{
	void *addr = shmat(g_shmid,NULL,0);
	if((void *) -1 == addr)
	{
		perror("shmat");
		return;
	}
	
	memcpy(s,addr,sizeof(Shm)); 

	shmdt(addr);

}

void get_music(const char *singer)
{
	struct json_object *obj = json_object_new_object();
	json_object_object_add(obj,"cmd",json_object_new_string("get_music_list"));
	json_object_object_add(obj,"singer",json_object_new_string(singer));

	socket_send_data(obj);
	
	char msg[1024] = {0};
	socket_recv_data(msg);

	create_link(msg);

	upload_music_list();



}

void get_volume(int *v)
{
    //获取音量大小
    if (ioctl(g_mixerfd, SOUND_MIXER_READ_VOLUME, v) == -1)
    {
        perror("ioctl");
        return;
    }
 
    *v /= 257;
}

int start_play()
{
	if(g_start_flag == 1)
	{
		return -1;
	}
	if(head->next == NULL)
	{
		return -1;
	}
	
	char name[32] = {0};
	strcpy(name,head->next->music_name);
	//printf("--------------------------------start_play:%s\n",name);	

	//初始化音量
	int volume = DFL_VOL;
	volume *= 257;
	ioctl(g_mixerfd, SOUND_MIXER_WRITE_VOLUME, &volume);

	g_start_flag = 1;

	play_music(name);
	return 0;

}

void play_music(char *n)
{
	pid_t child_pid = fork();
	if( -1 == child_pid )
	{
		perror("fork");
		return;
	}
	else if( 0 == child_pid)
	{
		close(0);
		signal(SIGUSR2,child_quit);
		child_process(n);
		exit(0);
	}
	else
	{
		return;
	}

}

void child_process(char *n)
{
	while(g_start_flag)
	{
		pid_t grand_pid = fork();
		if( -1 == grand_pid)
		{
			perror("fork");
			return;
		}
		else if( 0 == grand_pid)
		{
			close(0);
			
			Shm s;
			memset(&s,0,sizeof(s));
			
			if( strlen(n) == 0 )
			{
			
				grand_get_shm(&s);
				//printf("------------------------child_process--cur_music:%s\n",s.cur_music);
				if(find_next_music(s.cur_music,s.mode,n) == -1)
				{
					kill(s.parent_pid,SIGUSR1);
					kill(s.child_pid,SIGUSR2);
					usleep(100000);
					exit(0);
				}
			
			}

			char *arg[7] = {0};
			char music_path[128] = {0};

			strcpy(music_path,URL);
			strcat(music_path,n);
			
			arg[0] = "mplayer";
			arg[1] = music_path;
			arg[2] = "-slave";
			arg[3] = "-quiet";
			arg[4] = "-input";
			//arg[5] = "file=./cmd_fifo";
			arg[5] = "file=/dev/test/cmd_fifo";
			grand_get_shm(&s);
			
			char *p = n;
			while(*p != '/')
			{
				p++;
			}
			
			strcpy(s.cur_music,p+1);
			
			
			grand_set_shm(s);

#ifdef ARM
            		execv("/bin/mplayer", arg);
#else
            		execv("/usr/local/bin/mplayer", arg);
#endif
				
	
			
		}
		else
		{
			memset(n,0,sizeof(n));
			int status;
			wait(&status);

			usleep(100000);
		}
	}
	

}

void child_quit()
{

	g_start_flag = 0;

}

void grand_set_shm(Shm s)
{

	int shmid = shmget(SHMKEY, SHMSIZE, 0);
	if( -1 == shmid)
	{

		perror("grand shmget");
		return;
	}
	
	void *addr = shmat(shmid, NULL, 0);
	if( addr == (void *)-1 )
	{
		perror("grand shmat");
		return;
	}
	

	s.child_pid = getppid();
	s.grand_pid = getpid();
	
	
	memcpy(addr,&s,sizeof(s));

	shmdt(addr);


}

void grand_get_shm(Shm *s)
{
	int shmid = shmget(SHMKEY,SHMSIZE,0);
	if( -1 == shmid )
	{
		perror("grand shmid");
		return;
	}
	void *addr = shmat(shmid,NULL,0);
	if( (void *)-1 == addr)
	{
		perror("grand shmat");
		return;
	}
	
	memcpy(s,addr,sizeof(Shm));
	
	shmdt(addr);

}

void write_fifo(const char *cmd)
{
	int fd = open("/dev/test/cmd_fifo",O_WRONLY);
	//int fd = open("cmd_fifo",O_WRONLY);
	if( -1 == fd)
	{
		perror("open");
		return;
	}
	
	if(write(fd,cmd,strlen(cmd)) == -1)
	{
		perror("write");
		return;
	}
	close(fd);
}
void stop_play()
{
	if(g_start_flag == 0)
	{
		return;
	}

	Shm s;
	get_shm(&s);
	kill(s.child_pid,SIGUSR2);
	
	//write_fifo("stop\n");
	write_fifo("quit\n");

	int status;
	waitpid(s.child_pid,&status, 0);

	g_start_flag = 0;

}

void suspend_play()
{
	if(g_start_flag == 0 || g_suspend_flag == 1)
	{
		printf("--已暂停--\n");
		return;
	}
	printf("--暂停播放--\n");
	write_fifo("pause\n");

	g_suspend_flag = 1;
}

void continue_play()
{
	if(g_start_flag == 0 || g_suspend_flag == 0)
	{
		return;
	}
	printf("--继续播放--\n");
	write_fifo("pause\n");

	g_suspend_flag = 0;
}

void set_shm(Shm s)
{
	void *addr = shmat(g_shmid,NULL,0);
	if((void *)-1== addr)
	{
		perror("shmat");
		return;
	}
	memcpy(addr,&s,sizeof(s));
	shmdt(addr);
}

void next_play()
{
	if(g_start_flag == 0)
	{
		return;
	}
	Shm s;
	get_shm(&s);
	char music[128] = {0};
	if(find_next_music(s.cur_music,SEQUENCE,music) == -1)
	{
		stop_play();
		char singer[128] = {0};
		get_singer(singer);
		
		clear_link();
		get_music(singer);
		sleep(1);
		start_play();
		
		if(g_suspend_flag == 1)
		{
			g_suspend_flag == 0;
		}
		return;
	}
	
	char path[128] = {0};
	
	strcpy(path,URL);
	strcat(path,music);

	char cmd[256] = {0};
	sprintf(cmd,"loadfile %s\n",path);
	write_fifo(cmd);
	
	char *p = music;
	while(*p != '/')
	{
		p++;
	}
	strcpy(s.cur_music,p+1);
	set_shm(s);	
	if(g_suspend_flag == 1)
	{
		g_suspend_flag = 0;

	}
}

void prior_play()
{
	if(g_start_flag == 0)
	{
		return;
	}

	Shm s;
	get_shm(&s);

	char music[128] = {0};
	
	find_prior_music(s.cur_music,music);
	printf("-------%s\n",music);
	
	char path[128] = {0};
	strcpy(path,URL);
	strcat(path,music);
	
	char cmd[256] = {0};
	sprintf(cmd,"loadfile %s\n",path);
	write_fifo(cmd);
	
	char *p = music;
	while(*p != '/')
	{
		p++;
	}
	strcpy(s.cur_music,p+1);
	set_shm(s);

	if(g_suspend_flag == 1)
	{
		g_suspend_flag = 0;
	}
}

void voice_up()
{

	int volume;
	//获取音量大小
	if (ioctl(g_mixerfd, SOUND_MIXER_READ_VOLUME, &volume) == -1)
	{
		perror("ioctl");
		return;
	}

	volume /= 257;

	if (volume <= 95)
	{
		volume += 5;
	}
	else if (volume > 95 && volume < 100)
	{
		volume = 100;
	}
	else if (volume == 100)
	{
		printf("音量已经最大 ...\n");
		return;
	}

	volume *= 257;

	if (ioctl(g_mixerfd, SOUND_MIXER_WRITE_VOLUME, &volume) == -1)
	{
		perror("ioctl");
		return;
	}

	printf("--------------音量增加---------------\n");
}

void voice_down()
{

	int volume;
	//获取音量大小
	if (ioctl(g_mixerfd, SOUND_MIXER_READ_VOLUME, &volume) == -1)
	{
		perror("ioctl");
		return;
	}

	volume /= 257;

	if (volume >= 5)
	{
		volume -= 5;
	}
	else if (volume > 0 && volume < 5)
	{
		volume = 0;
	}
	else if (volume == 0)
	{
		printf("音量已经最小 ...\n");
		return;
	}

	volume *= 257;

	if (ioctl(g_mixerfd, SOUND_MIXER_WRITE_VOLUME, &volume) == -1)
	{
		perror("ioctl");
		return;
	}
	printf("--------------音量减小---------------\n");
}
	
void circle_play()
{
	Shm s;
	get_shm(&s);
	s.mode = CIRCLE;
	set_shm(s);
	printf("------单曲循环----\n");
}

void sequence_play()
{
	Shm s;
	get_shm(&s);
	s.mode = SEQUENCE;
	set_shm(s);
	printf("-----顺序播放-----\n");
		
}

void singer_play(const char *singer)
{
	stop_play();
	clear_link();
	get_music(singer);
	start_play();
}



