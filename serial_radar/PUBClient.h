#pragma once
#include <Windows.h>
#define HAVE_STRUCT_TIMESPEC
#include <string>
#include <queue>
#include <iostream>
#include "pthread.h"
#include "json.hpp"

using namespace std;

class PUBClient
{
public:
	PUBClient();
	~PUBClient();

	void init();
	void *run();
	void open();
	void close();
	void setDebugMsg(bool onoff);
	void setBroker(string parm);
	string getBroker();
	void setPort(string parm);
	string getPort();
	void setClient(string parm);
	string getClient();
	void setTopic(string parm);
	string getTopic();
	void setMaxQSize(int parm);
	void publish(string topic, string lastwill, char *payload);
	void setNotifyCycle(int parm);
	void sendHeartBeat();
	void setHeartOccQSize(int i);
	void setHeartOccQMax(int i);
	void setHeartPUBSUBQSize(int i);
	void setHeartPUBSUBQMax(int i);

	static void *context_run(void *context)
	{
		return ((PUBClient *)context)->run();
	}
private:
	size_t max_q_size =5;
	unsigned int max = 0;
	bool available=false;
	string broker;
	string port;
	string client;
	string topic;
	typedef struct {
		string broker;
		string port;
		string client;
		string topic;
		string lastwill;
		string payload;
	} q_item_t;

	void PUBClient::send(q_item_t *item);

	bool addToQueue(q_item_t *item);
	q_item_t * removeFromQueue();

	queue<q_item_t *> sendq;
	int notifyCycle = 0;
	pthread_mutex_t qlock;
	bool debugMsg;
	int PUBSUBQSize;
	int PUBSUBQMax;
	int OccQSize;
	int OccQMax;
};

