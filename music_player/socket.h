#ifndef __SOCKET_H
#define __SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <json/json.h>
#include "player.h"
#include "device.h"


#define PORT 8000
#define IP "8.134.62.10"
//#define IP "127.0.0.1"

int init_socket();

void upload_music_list();

void send_server(int sig);

void socket_send_data(struct json_object *obj);

void socket_recv_data(char *msg);

void socket_start_play();

void socket_stop_play();

void socket_suspend_play();

void socket_continue_play();

void socket_next_play();

void socket_prior_play();

void socket_voice_up();

void socket_voice_down();

void socket_circle_play();

void socket_sequence_play();

#endif
