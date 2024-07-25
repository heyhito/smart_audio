#ifndef __PLAYER_H
#define __PLAYER_H

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/soundcard.h>
#include "socket.h"
#include "link.h"

#define SHMKEY 1234
#define SHMSIZE 4096

#define DFL_VOL 	60	

#define SEQUENCE	1
#define CIRCLE		2
#define URL		"http://8.134.62.10/music/"

typedef struct Shm
{
	pid_t parent_pid;
	pid_t child_pid;
	pid_t grand_pid;
	char cur_music[128];
	int mode;
}Shm;

int init_shm();

void get_shm(Shm *s);
void set_shm(Shm s);
void get_volume(int *v);
void get_music(const char *singer);
int start_play();
void play_music(char *n);
void child_process(char *n);
void grand_get_shm(Shm *s);
void grand_set_shm(Shm s);
void child_quit();
void stop_play();
void write_fifo(const char *cmd);
void suspend_play();
void continue_play();
void next_play();
void prior_play();
void voice_up();
void voice_down();
void circle_play();
void sequence_play();
void singer_play(const char *singer);

#endif
