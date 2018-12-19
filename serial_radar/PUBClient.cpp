#include "json.hpp"
#include "PUBClient.h"
#include "async_publish.h"
#include "Config.h"

using namespace nlohmann;


PUBClient::PUBClient()
{
	pthread_mutex_init(&qlock, nullptr);

}


PUBClient::~PUBClient()
{
}

void *PUBClient::run() {
	uint32_t heartbeat = 0;
	int notify = 0;
	while (1) {
		if (notify++ < notifyCycle) {
			continue;
		}
		while (true) {
			q_item_t * item = removeFromQueue();
			if (nullptr != item) {
				send(item);
				delete(item);
			} else 
			{
				break;
			}

		}
		if (heartbeat++ > 20) {
			sendHeartBeat();
			heartbeat = 0;
		}
		Sleep(50 * notifyCycle);
	}
}
//“yyyy-mm-ddThh:mm:ss
void PUBClient::sendHeartBeat() {
	json j;
	time_t now;
	q_item_t *item = new q_item_t;
	item->broker.append(broker);
	item->topic.append(this->topic);
	item->topic.append("/heartbeat");
	item->port.append(port);
	item->client.append(client);
	item->lastwill.append("hereNoMore");
	try {
		time(&now);
		char buf[sizeof "2011-10-08T07:07:09Z"];
		strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
		j[JS_TIME] = buf;
		j[JS_STATUS] = "going strong";
		j[JS_MSG_OCCQ_SIZE] = this->OccQSize;
		j[JS_MSG_OCCQ_MAX] = this->OccQMax;
		j[JS_MSG_PUBSUBQ_SIZE] = this->PUBSUBQSize;
		j[JS_MSG_PUBSUBQ_MAX] = this->PUBSUBQMax;
		item->payload.append(j.dump());
		this->PUBSUBQSize = 0;
		max = 0;
	}
	catch (exception e) {
		cout << "ERROR " << e.what() << endl;
	}
	if (!addToQueue(item)) delete item;
}
void PUBClient::open() {

}
void PUBClient::close() {

}

void PUBClient::setDebugMsg(bool onoff) {
	this->debugMsg = onoff;
}
void PUBClient::setNotifyCycle(int parm) {
	notifyCycle = parm;
}

void PUBClient::setBroker(string parm) {
	broker = parm;
}
string PUBClient::getBroker()
{
	return broker;
}
void PUBClient::setPort(string parm)
{
	port = parm;
}
string PUBClient::getPort()
{
	return port;

}

void PUBClient::setClient(string parm) {
	client = parm;
}
string PUBClient::getClient()
{
	return client;
}

void PUBClient::setTopic(string parm) {
	topic = parm;
}
string PUBClient::getTopic() {
	return topic;
}

void PUBClient::publish(string topic, string lastwill, char *payload) {
	q_item_t * item = new q_item_t();
	item->broker = getBroker();
	item->client = getClient();
	item->port = getPort();
	item->topic = this->topic;
	item->topic.append(topic);
	item->lastwill = lastwill;
	item->payload.append(payload);
	if (!addToQueue(item)) delete item;
}

void PUBClient::send(q_item_t *item) {
	if (debugMsg) {
		cout << "topic : " << item->topic.c_str() << " p: " << item->payload.c_str() << endl;
	}
	if (async_publish((char *)item->broker.c_str()
		, nullptr, (char *)item->topic.c_str()
		, (char *)item->payload.c_str()
		, (char *)item->lastwill.c_str(), 0)) {
		Sleep(30);
	}
	else {
		cout << "error publishing mqtt message" << endl;
	}
}
bool PUBClient::addToQueue(q_item_t *item) {

	if (sendq.size() >= max_q_size) {
		// make some room
		for (int i = 0; i < max_q_size / 2; i++) {
			q_item_t * item = removeFromQueue();
			if (nullptr != item) {
				delete (item);
			}
		}
		//return false;
	}
	this->setHeartPUBSUBQMax(max_q_size);
	this->setHeartPUBSUBQSize(max);

	if (this->sendq.size() > max) {
		max = this->sendq.size();
		this->setHeartPUBSUBQMax(max_q_size);
		this->setHeartPUBSUBQSize(max);
	}
	pthread_mutex_lock(&qlock);
	sendq.push(item);
	PUBClient::available = true;
	pthread_mutex_unlock(&qlock);
	return true;
}
PUBClient::q_item_t * PUBClient::removeFromQueue() {
	pthread_mutex_lock(&qlock);
	PUBClient::q_item_t * item = nullptr;
	if (!sendq.empty()) {
		PUBClient::available = true;
		item = sendq.front();
		sendq.pop();
	}
	else {
		PUBClient::available = false;
	}
	pthread_mutex_unlock(&qlock);
	return item;
}
void PUBClient::setMaxQSize(int parm)
{
	max_q_size = parm;
}

void PUBClient::setHeartOccQSize(int i) {
	this->OccQSize = i;
}
void PUBClient::setHeartOccQMax(int i) { 
	this->OccQMax = i; 
}
void PUBClient::setHeartPUBSUBQSize(int i) { 
	this->PUBSUBQSize = i;
}
void PUBClient::setHeartPUBSUBQMax(int i){
	this->PUBSUBQMax = i;
}


