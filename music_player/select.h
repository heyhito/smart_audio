#ifndef __SELECT_H
#define __SELECT_H
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "player.h"
#include "device.h"
#include "socket.h"

void init_select();

void m_select();

void select_read_stdio();

void select_read_button();

void select_read_serial();

void select_read_socket();

int parse_message(char *msg,char *cmd);

#endif

