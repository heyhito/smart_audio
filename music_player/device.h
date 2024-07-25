#ifndef __DEVICE_H
#define __DEVICE_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>


int init_device();
void start_buzzer();
int get_key_id();




#endif
