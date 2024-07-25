#include "device.h"

extern int g_maxfd;
extern fd_set READSET;

int g_mixerfd = 0;
int g_buttonfd = 0;
int g_serialfd = 0;
int g_buzzerfd = 0;

int init_device()
{
	//初始化音频设备 （ubuntu不适用）
	
	g_mixerfd = open("/dev/mixer", O_RDWR);
	if (-1 == g_mixerfd)
	{
		perror("open mixer");
		return -1;
	}

	g_maxfd = (g_maxfd < g_mixerfd) ? g_mixerfd : g_maxfd;
	
	
#ifdef ARM
	g_buttonfd = open("/dev/buttons", O_RDONLY);
	if(-1 == g_buttonfd)
	{
		perror("open button");
		return -1;
	}
	
	g_maxfd = ( g_maxfd < g_buttonfd) ? g_buttonfd : g_maxfd;
	FD_SET(g_buttonfd,&READSET);
	
	printf("按键初始化成功\n");

	g_serialfd = open("/dev/s3c2410_serial1",O_RDONLY);
	//g_serialfd = open("/dev/ttySAC1",O_RDONLY);
	if(-1 == g_serialfd)
	{
		perror("open serial");
		return -1;
	}
	if( -1 == init_serial() )
	{
		printf("串口初始化失败\n");
		return -1;
	}
	
	g_maxfd = ( g_maxfd < g_serialfd) ? g_serialfd : g_maxfd;
	FD_SET(g_serialfd,&READSET);
	printf("串口初始化成功\n");

	g_buzzerfd = open("/dev/pwm",O_RDONLY);
	if(-1 == g_buzzerfd)
	{
		perror("open buzzerfd");
		return -1;
	}

	g_maxfd = ( g_maxfd < g_buzzerfd) ? g_buzzerfd : g_maxfd;
	printf("蜂鸣器初始化成功\n");
#endif
	return 0;

}

void start_buzzer()
{
	int freq = 1000;
	int ret = ioctl(g_buzzerfd, 1, freq);
	if (-1 == ret)
	{
		perror("ioctl");
		return;
	}

	usleep(500000);

	ret = ioctl(g_buzzerfd, 0);
	if (-1 == ret)
	{
		perror("ioctl");
	}
}

int get_key_id()
{
	char buttons[6] = {'0', '0', '0', '0', '0', '0'};   
	char cur_buttons[6] = {0};

	ssize_t size = read(g_buttonfd, cur_buttons, 6);
	if (-1 == size)
	{
		perror("read");
		return -1;
	}

	int i;
	for (i = 0; i < 6; i++)
	{
		if (buttons[i] != cur_buttons[i])
		{
		    return i + 1;
		}
	}

	return -1;
}
		
int init_serial()
{
	struct termios TtyAttr;

	memset(&TtyAttr, 0, sizeof(struct termios));
	TtyAttr.c_iflag = IGNPAR;
	TtyAttr.c_cflag = B115200 | HUPCL | CS8 | CREAD | CLOCAL;
	TtyAttr.c_cc[VMIN] = 1;

	if (tcsetattr(g_serialfd, TCSANOW, &TtyAttr) == -1)
	{
		perror("tcsetattr");
		return -1;
	}
}


