#pragma once
#include <Windows.h>
#define HAVE_STRUCT_TIMESPEC
#include "serial_radarAPI_if.h"
#include <string>
#include <queue>
#include <iostream>
#include <inttypes.h>
#include "json.hpp"
#include "pthread.h"
#include "PUBClient.h"

using namespace nlohmann;
using namespace std;


class Occupancy
{
public:
	Occupancy(bool debugmsg);
	virtual ~Occupancy();

	#define OCC_HANDLE VOID *
private:
	struct occ_device {
		OCC_HANDLE handle;
		string name;
		std::vector<std::vector<int>> occRefCounts;
	};

	typedef struct {
		PUBClient *pubclient;
		iSYSTargetList *targetList;
		string topic;
		string lastwill;
		float32_t adjx;
		float32_t adjy;
		float32_t adjAngle;
		occ_device *device;
	} occ_item_t;

private:

	int angleSegmentSize; // degrees
	int rangeSegmentSize; // meter
	int cellExpired;
	int cellMax;
	int rangeMax;
	int rangeMin;
	int angleMax;
	int angleMin;
	string maxVelocity;
	queue<occ_item_t *> sendq;
	pthread_mutex_t qlock;

	size_t max_q_size = 5;
	unsigned int max = 5;
	bool available = false;
	string broker;
	string port;
	string client;
	PUBClient *pub_client;
	void formatMQTTMessage(iSYSTargetList *targetList, string &sb,string &nearest, uint16_t &items
		, float32_t adjx, float32_t adjy, float32_t adjAngle);
	void formatTheList(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items);

public:
	void setPUBClient(PUBClient *parm);

	void init();
	void *run();
	void open();
	void pause();
	void update();
	void setDebugMsg(bool onoff);
	void setMaxQSize(int parm);
	void publish(OCC_HANDLE handle, PUBClient *pub_client
		, iSYSTargetList *targetList
		, string topic, string lastwill, float32_t adjx, float32_t adjy, float32_t adjAngle);
	void notify(occ_item_t *item, char desc[], bool print);
	OCC_HANDLE addDevice(string name);
	static void *context_run(void *context)
	{
		return ((Occupancy *)context)->run();
	}

	void setRangeMax(int size);
	void setRangeMin(int size);
	void setAngleMax(int size);
	void setRangeSegmentSize(int size);
	void setAngleSegmentSize(int size);
	void setCellExpired(int size);
	void setCellMax(int val);
	void setMaxVelocity(string parm);

	int getRangeMax();
	int getRangeMin();
	int getAngleMax();

	int getRangeSegmentSize();
	int getAngleSegmentSize();
	int getCellExpired();
	int getCellMax();
	string getMaxVelocity();


private:
	occ_device *Devices[4] = { 0 };
	int nextHandle=0;
	void buildRefCountVector(std::vector<std::vector<int>> &occRefCounts);
	void updateGrid(occ_device *dev, iSYSTargetList *targetList);
	void Occupancy::updateGridData(occ_device *dev);
	void Occupancy::clearGridData(occ_device *dev);
	int occ_num_range_segments;
	int occ_num_angle_segments;
	int occ_half_angles;
	bool addToQueue(occ_item_t *item);
	occ_item_t * removeFromQueue();


	json j_nearest;
	json j_nearest_array;
	bool debugMsg;
};
