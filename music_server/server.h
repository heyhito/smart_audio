#ifndef __SERVER_H
#define __SERVER_H


#include <event.h>          // 包含 libevent 库的头文件，用于事件处理
#include <event2/listener.h>       // 包含 libevent 库的监听器头文件，用于网络连接监听
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <list>
#include <time.h>
#include <json/json.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "database.h"
#include "player.h"

#define IP "172.29.249.51"
#define PORT 8000
#define SEQUENCE 1
#define CIRCLE   2


class Server
{
private:
	struct event_base *m_base;
	DataBase *m_database;
	Player *m_p;

public:
	Server();
	~Server();
	
	void listen(const char *ip,int port);
	struct event_base *server_get_base();
	void server_read_data(struct bufferevent *,char *);
	void server_get_music(struct bufferevent *,std::string);
	void server_send_data(struct bufferevent *,Json::Value &);
	void server_player_handle(struct bufferevent *, Json::Value &);
	void server_start_time();
	void server_app_register(struct bufferevent *, Json::Value &);
	void server_app_login(struct bufferevent *, Json::Value &);
	void server_app_bing(struct bufferevent *, Json::Value &);
	void server_client_offline(struct bufferevent *);
	void server_app_offline(struct bufferevent *);

	static void listener_cb(struct evconnlistener *listener,evutil_socket_t fd, struct sockaddr *addr, int socklen, void *arg);
	static void read_cb(struct bufferevent *,void *);
	static void event_cb(struct bufferevent *,short ,void *);
	static void timeout_cb(evutil_socket_t,short,void *);
};


#endif
