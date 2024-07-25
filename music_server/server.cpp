#include "server.h"
#include <iostream>

Server::Server()
{
	m_base = event_base_new();
	
	m_database = new DataBase;
	if(!m_database->database_init_table())
	{
		std::cout << "数据库初始化失败" << std::endl;
		exit(1);
	}

	m_p = new Player;
	
}


Server::~Server()
{
	if(m_database)
	{
		delete m_database;
	}
	if(m_p)
	{
		delete m_p;
	}

}

void Server::listen(const char *ip,int port)
{

	struct sockaddr_in server_info;
	int len = sizeof(server_info);
	memset(&server_info,0,len);
	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(port);
	server_info.sin_addr.s_addr = inet_addr(ip);

	struct evconnlistener *listener = evconnlistener_new_bind(
					m_base,
					listener_cb,
					this,
					LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
					5,
					(struct sockaddr *)&server_info,
					len);
	if( NULL == listener)
	{
		std::cout << "bind error" << std::endl;
		return;
	}

	server_start_time();


	evconnlistener_free(listener);
	event_base_free(m_base);

}

void Server::server_start_time()
{
	struct event tv;
	struct timeval t;

	if(event_assign(&tv,m_base,-1,EV_PERSIST,timeout_cb,m_p) == -1)
	{
		std::cout << "event_assign error" << std::endl;

	}

	evutil_timerclear(&t);
	t.tv_sec = 2;
	event_add(&tv,&t);

	event_base_dispatch(m_base);
}

void Server::timeout_cb(evutil_socket_t fd,short,void *arg)
{
	Player *p = (Player *)arg;
	p->player_traverse_list();

}
	
void Server::listener_cb(struct evconnlistener *l, evutil_socket_t fd, struct sockaddr *c,
                            int cosklen, void *arg)
{
	struct sockaddr_in *client_info = (struct sockaddr_in *)&c;

	std::cout << "[new client connection] ";
	std::cout << inet_ntoa(client_info->sin_addr) << ":";
	std::cout << client_info->sin_port << std::endl;

	Server *s = (Server *)arg;
	struct event_base *base = s->server_get_base();

	struct bufferevent *bev = bufferevent_socket_new(base,fd,BEV_OPT_CLOSE_ON_FREE);
	if( NULL == bev )
	{
		std::cout << "bufferevent_socket_new error" << std::endl;
		exit(1);
	}

	bufferevent_setcb(bev,read_cb,NULL,event_cb,s);
	bufferevent_enable(bev,EV_READ);
	
}


void Server::read_cb(struct bufferevent *bev,void *ctx)
{

	char buf[1024] = {0};

	Server *s = (Server *)ctx;
	s->server_read_data(bev,buf);

	Json::Reader reader;
	Json::Value value;

	if(!reader.parse(buf,value))
	{
		std::cout << "[JSON PARSE ERROR]" << std::endl;
		return;
	}

	//std::cout << value << std::endl;
	
	if(value["cmd"] == "get_music_list")
	{
		s->server_get_music(bev,value["singer"].asString());
	}
	else if(value["cmd"] == "app_register")
	{
		s->server_app_register(bev,value);
	}
	else if(value["cmd"] == "app_login")
	{
		s->server_app_login(bev,value);
	}
	else if(value["cmd"] == "app_bind")
	{
		s->server_app_bing(bev,value);
	}
	else if(value["cmd"] == "app_offline")
	{
		s->server_app_offline(bev);
	}
	else
	{
		s->server_player_handle(bev,value);
	}


}


void Server::server_app_offline(struct bufferevent *bev)
{
	m_p->player_app_offline(bev);
}

void Server::server_app_bing(struct bufferevent *bev,Json::Value &v)
{
	Json::Value val;
	std::string appid = v["appid"].asString();
	std::string deviceid = v["deviceid"].asString();

	m_database->database_connect();

	m_database->database_bind_user(appid,deviceid);

	m_database->database_disconnect();

	val["cmd"] = "app_bind_reply";
	val["result"] = "success";

	server_send_data(bev,val);
}

void Server::server_app_login(struct bufferevent *bev,Json::Value &v)
{
	Json::Value val;
	std::string appid = v["appid"].asString();
	std::string password = v["password"].asString();
	std::string deviceid;
	

	m_database->database_connect();
	do
	{
		if(!m_database->database_user_exist(appid))
		{
			val["result"] = "user_not_exist";
			break;
		}

		if(!m_database->database_password_correct(appid,password))
		{
			val["result"] = "password_error";
			break;
		}
		if(m_database->database_user_bind(appid,deviceid))
		{
			val["result"] = "bind";
			val["deviceid"] = deviceid;
		
		}
		else
		{
			val["result"] = "not_bind";
		
		}

	}while(0);

	m_database->database_disconnect();

	val["cmd"] = "app_login_reply";
	
	server_send_data(bev,val);
}

void Server::server_app_register(struct bufferevent *bev, Json::Value &v)
{
	Json::Value val;

	m_database->database_connect();

	std::string appid = v["appid"].asString();
	std::string password = v["password"].asString();

	if(m_database->database_user_exist(appid))
	{
		val["result"] = "failure";
		std::cout << "[用户存在，注册失败]" << std::endl;
		
	}
	else
	{
		m_database->database_add_user(appid,password);
		val["result"] = "success";
		std::cout << "[用户注册成功]" << std::endl;
	}

	m_database->database_disconnect();

	val["cmd"] = "app_register_reply";

	server_send_data(bev,val);
}


void Server::server_player_handle(struct bufferevent *bev,Json::Value &v)
{
	if(v["cmd"] == "info")
	{
		m_p->player_update_list(bev,v,this);
	}
	else if(v["cmd"] == "app_info")
	{
		m_p->player_app_list(bev,v);
	}
	else if(v["cmd"] == "upload_music")
	{
		m_p->player_upload_music(this,bev,v);
	}
	else if(v["cmd"] == "app_get_music")
	{
		m_p->player_get_music(this,bev,v);
	}
	else if(v["cmd"] == "app_start" || v["cmd"] == "app_stop" ||
            v["cmd"] == "app_suspend" || v["cmd"] == "app_continue" ||
            v["cmd"] == "app_prior" || v["cmd"] == "app_next" ||
            v["cmd"] == "app_voice_up" || v["cmd"] == "app_voice_down" ||
            v["cmd"] == "app_circle" || v["cmd"] == "app_sequence")
    	{
        /*
        参数：
        this:需要用到server类的server_send_data函数
        bev:这个bev和前面的bev不一样，这个bev是APP的事件，因为这些指令是APP发送的
        */
        	m_p->player_option(this,bev,v);
    	}
    	else if (v["cmd"] == "app_start_reply" ||
             v["cmd"] == "app_stop_reply" ||
             v["cmd"] == "app_suspend_reply" ||
             v["cmd"] == "app_continue_reply" ||
             v["cmd"] == "app_prior_reply" ||
             v["cmd"] == "app_next_reply" ||
             v["cmd"] == "app_voice_up_reply" || 
             v["cmd"] == "app_voice_down_reply" || 
             v["cmd"] == "app_circle_reply" ||
             v["cmd"] == "app_sequence_reply")
    	{
        	m_p->player_reply_option(this, bev, v);
    	}
}



void Server::server_get_music(struct bufferevent *bev, std::string s)
{

	Json::Value val;
	Json::Value arr;
	
	std::list<std::string> l;
	
	char path[128] = {0};

	sprintf(path,"/var/www/html/music/%s",s.c_str());

	DIR *dir = opendir(path);
	if(NULL == dir)
	{
		perror("opendir");
		return;
	}
	struct dirent *d;
	while((d = readdir(dir)) != NULL)
	{
		if(d->d_type != DT_REG)
			continue;
		if(!strstr(d->d_name,".mp3"))
			continue;
		char name[1024] = {0};
		sprintf(name,"%s/%s",s.c_str(),d->d_name);

		l.push_back(name);
	
	}

	auto it = l.begin();
	srand(time(NULL));
	int count = rand() % (l.size() -4);
	for(int i = 0; i < count; i++)
	{
		it++;
	}
	for(int i = 0; i < 5 && it != l.end(); i++,it++)
	{
		arr.append(*it);
	}

	val["cmd"] = "reply_music";
	val["music"] = arr;

	server_send_data(bev,val);


}

void Server::server_send_data(struct bufferevent *bev,Json::Value &v)
{
	char msg[1024] = {0};
	
	std::string SendStr = Json::FastWriter().write(v);
//	std::cout << "SendStr : " << SendStr << std::endl;
	int len = SendStr.size();
	memcpy(msg,&len,sizeof(int));
	memcpy(msg + sizeof(int),SendStr.c_str(),len);

	if(bufferevent_write(bev,msg,len + sizeof(int)) == -1)
	{
		std::cout << "bufferevent_write error" << std::endl;
	}

	

}


void Server::server_read_data(struct bufferevent *bev,char *msg)
{
	char buf[8] = {0};
	size_t size = 0;

	while(true)
	{
		size += bufferevent_read(bev,buf + size,4 - size);
		if( size >= 4)
			break;
	}

	int len = *(int *)buf;
	size = 0;
	while(true)
	{
		size += bufferevent_read(bev, msg + size, len - size);
		if(size >= len)
			break;
	}
	
	std::cout << "--LEN " << len << " MSG " << msg << std::endl;

}	

void Server::event_cb(struct bufferevent *bev,short what, void *ctx)
{
	Server *s = (Server *)ctx;
	if(what & BEV_EVENT_EOF)
	{
		s->server_client_offline(bev);
	}
}

void Server::server_client_offline(struct bufferevent *bev)
{
	m_p->player_offline(bev);
}


struct event_base *Server::server_get_base()
{
	return m_base;
}

