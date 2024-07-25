#include "player.h"
#include "server.h"

Player::Player()
{

	info = new std::list<PlayerInfo>;

}


Player::~Player()
{
	if(info)
	{
		delete info;
	}
}

void Player::player_update_list(struct bufferevent *bev,Json::Value &v,Server *s)
{
	auto it = info->begin();
	for(; it != info->end();it++)
	{
		if(it->deviceid == v["deviceid"].asString())
		{
			debug("设备已存在，更新链表信息...");
			it->cur_music = v["cur_music"].asString();
			it->volume = v["volume"].asInt();
			it->mode = v["mode"].asInt();
			it->d_time = time(NULL);

			if(it->a_bev)
			{
				debug("APP在线，数据转发给APP...");
				s->server_send_data(it->a_bev,v);
			}

			return;
		}
	}

	PlayerInfo p;
	p.deviceid = v["deviceid"].asString();
	p.cur_music = v["cur_music"].asString();
	p.volume = v["volume"].asInt();
	p.mode = v["mode"].asInt();
	p.d_time = time(NULL);
	p.d_bev = bev;
	p.a_bev = NULL;

	info->push_back(p);
	
	debug("第一次上报数据，建立结点...");

	

}

void Player::player_app_list(struct bufferevent *bev,Json::Value &v)
{
	auto it = info->begin();
	for(;it!=info->end();it++)
	{
		if(it->deviceid == v["deviceid"].asString())
		{
			it->appid = v["appid"].asString();
			it->a_time = time(NULL);
			it->a_bev = bev;

		}
	}
}

void Player::player_traverse_list()
{
	debug("定时器事件：遍历链表");
	auto it = info->begin();
	for(;it!= info->end();it++)
	{
		if(time(NULL) - it->d_time > 6)
		{
			info->erase(it);
		}

		if(it->a_bev)
		{
			if(time(NULL) - it->d_time > 6)
			{
				it->a_bev = NULL;

			}
		}

	}
}

void Player::debug(const char *s)
{

	time_t cur = time(NULL);
	struct tm *t = localtime(&cur);

	std::cout << "[ " << t->tm_hour << " : " << t->tm_min << " : ";
	std::cout << t->tm_sec << " ] " << s << std::endl;


}


void Player::player_upload_music(Server *s,struct bufferevent *bev,Json::Value &v)
{

	auto it = info->begin();
	for(;it!=info->end();it++)
	{
		if(it->d_bev == bev)
		{
			if(it->a_bev)
			{
				s->server_send_data(it->a_bev,v);
				debug("APP在线，歌曲名字转发成功");
			}
			else
			{
				debug("APP不在线，歌曲名字不转发");
			}
			break;
		}
	}

}

void Player::player_option(Server *s,struct bufferevent *bev,Json::Value &v)
{
	auto it = info->begin();
	for(;it!=info->end();it++)
	{
		if(it->a_bev == bev)
		{
			s->server_send_data(it->d_bev,v);
			debug("音箱在线，指令转发成功");
			return;
		}
	}

	Json::Value value;
	std::string cmd = v["cmd"].asString();
	cmd += "_reply";
	value["cmd"] = cmd;
	value["result"] = "offline";
	s->server_send_data(bev,value);
	debug("音箱不在线 转发失败");

}

void Player::player_reply_option(Server *s,struct bufferevent *bev,Json::Value &v)
{
	auto it = info->begin();
	for (; it != info->end(); it++)
    	{
        	if (it->d_bev == bev)
        	{	
            		if (it->a_bev)
            		{
                		s->server_send_data(it->a_bev, v);
               			break;
            		}	
        	}
    	}	
}


void Player::player_offline(struct bufferevent *bev)
{
	auto it = info->begin();
	for(;it!=info->end();it++)
	{
		if(it->d_bev == bev)
		{
			debug("音箱异常下线处理");
			info->erase(it);
			break;

		}

		if(it->a_bev = bev)
		{
			debug("APP异常下线处理");
			it->a_bev = NULL;
			break;
		}
	}
}


void Player::player_app_offline(struct bufferevent *bev)
{
	auto it = info->begin();
	for(;it!=info->end();it++)
	{
		if(it->a_bev == bev)
		{
			debug("APP正常下线处理");
			it->a_bev = NULL;
			break;
		}
	}
}

void Player::player_get_music(Server *s,struct bufferevent *bev,Json::Value &v)
{
	auto it = info->begin();
	for(;it!=info->end();it++)
	{
		if(it->a_bev == bev)
		{
			s->server_send_data(it->d_bev,v);
		}
	
	}

}
