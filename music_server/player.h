#ifndef __PLAYER_H
#define __PLAYER_H
#include <list>
#include <time.h>
#include <iostream>
#include <event.h>
#include  <json/json.h>
#include <string.h>

class Server;

struct PlayerInfo
{
	std::string deviceid;
	std::string appid;
	std::string cur_music;
	int volume;
	int mode;
	time_t d_time;
	time_t a_time;

	struct bufferevent *d_bev;
	struct bufferevent *a_bev;
};

class Player
{
private:

	std::list<PlayerInfo> *info;

public:
	Player();
	~Player();
	
	void player_update_list(struct bufferevent *,Json::Value &,Server *);
	void player_app_list(struct bufferevent *, Json:: Value &);
	void player_traverse_list();
	void player_upload_music(Server *,struct bufferevent *,Json::Value &);
	void player_option(Server *, struct bufferevent *, Json::Value &);
	void player_reply_option(Server *,struct bufferevent *,Json::Value &);
	void debug(const char *);
	void player_offline(struct bufferevent *);
	void player_app_offline(struct bufferevent *);
	void player_get_music(Server *,struct bufferevent *,Json::Value &);


};


#endif

