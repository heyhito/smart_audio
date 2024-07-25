#include "link.h"


extern int g_start_flag;
extern Node *head;

int init_link()
{
	head = (Node *)malloc(sizeof(Node) * 1);
	if(head == NULL)
	{
		return -1;
	}
	
	head->next = NULL;
	head->prior = NULL;
	
	return 0;
}

void create_link(const char *s)
{

	struct json_object *obj = (struct json_object *)json_tokener_parse(s);
	if(NULL == obj)
	{
		printf("不是一个json格式的字符串\n");
		return;
	}

	struct json_object *array;
	array = (struct json_object *)json_object_object_get(obj,"music");

	int i = 0;
	for(i;i<5;i++)
	{
		struct json_object *music = (struct json_object *)json_object_array_get_idx(array,i);
		if(-1 == insert_link((const char *)json_object_get_string(music)))
		{
			printf("插入链表失败\n");
			return;
		}
		json_object_put(music);
	}
	json_object_put(obj);
	json_object_put(array);

}



int insert_link(const char *name)
{

	Node *p = head;
	while(p->next)
		p = p->next;
	
	Node *new = (Node *)malloc(sizeof(Node));
	if(NULL == new)
	{
		printf("malloc failure\n");
		return -1;
	}
	
	strcpy(new->music_name,name);
	new->next = NULL;
	new->prior = p;
	p->next = new;

	return 0;
}

void traverse_link()
{

	Node *p = head->next;
	
	while(p)
	{
		printf("%s\n",p->music_name);
		p = p->next;
	}

}

void clear_link()
{
	Node *p = head->next;
	while(p)
	{
		head->next = p->next;
		free(p);
		p = head->next;
	}
}

int find_next_music(char *cur,int mode, char *next)
{

	Node *p = head->next;
	while(p)
	{
		if(strstr(p->music_name,cur) != NULL)
		{
			break;
		}
		p = p->next;
	}

	if(mode == CIRCLE)
	{
		strcpy(next,p->music_name);
		return 0;
	}
	else if(mode == SEQUENCE)
	{
		if(p->next != NULL)
		{
			strcpy(next,p->next->music_name);	
			return 0;
		}
		else
		{

			return -1;

		}
	}

}

void find_prior_music(char *cur, char *prior)
{

	if(cur == NULL || prior == NULL)
	{
		return;
	}
	Node *p = head->next;
	if(strstr(p->music_name,cur))
	{
		strcpy(prior,p->music_name);
		return;
	}
	p = p->next;
	while(p)
	{
		if(strstr(p->music_name,cur))
		{
			strcpy(prior,p->prior->music_name);
			return;
		}
		p = p->next;
	}
	printf("遍历链表失败\n");
}	

void update_music()
{
	g_start_flag = 0;
	
	Shm s;
	get_shm(&s);
	int status;
	waitpid(s.child_pid,&status,0);

	char singer[128] = {0};
	get_singer(singer);

	clear_link();
	
	get_music(singer);

	start_play();
}

void get_singer(char *s)
{
	if(head->next == NULL)
	{
		return;
	}
	char *begin = head->next->music_name;
	char *p = begin;
	while(*p != '/')
	{
		p++;
	}
	strncpy(s, begin, p - begin);

}
