#include "Occupancy.h"
#include "PUBClient.h"
#include "serial_radarAPI_if.h"
#include "Config.h"


Occupancy::Occupancy(bool debug)
{
	pthread_mutex_init(&qlock, nullptr);
	debugMsg = debug;
}


Occupancy::~Occupancy()
{
}

bool stop = false;
bool updating = false;
void *Occupancy::run() {
	int update_loop = 0;
	int clear_loop = 0;
	while (!stop) {
		occ_item_t * item = removeFromQueue();
		if (nullptr == item) {
			Sleep(50);
			if (++update_loop > 15) {
				for (int i = 0; i < 4; i++) {
					if (nullptr != Devices[i]) {
						updateGridData(Devices[i]);
					}
				}
				update_loop = 0;
			}
		}
		else {
			if (!updating) {
				updateGrid(item->device, item->targetList);
			}
			notify(item, "try", false);
			delete(item->targetList);
		}
		Sleep(50);
	}
	return this;
}

void Occupancy::init() {
	// create defaults
	this->setMaxQSize(60);
	this->setAngleMax(160);
	this->setRangeMax(8);
	this->setRangeMin(0);
	this->setRangeSegmentSize(1);
	this->setAngleSegmentSize(14);
	this->setCellMax(255);
	this->setCellExpired(128);
	this->setMaxVelocity("5.0");
}

void Occupancy::open() {

}

void Occupancy::update() {
	updating = false;
}
void Occupancy::setDebugMsg(bool onoff) {
	debugMsg = onoff;
}

void Occupancy::pause() {
	updating = true;
}

OCC_HANDLE Occupancy::addDevice(string name) {

	occ_device *d = new occ_device();
	OCC_HANDLE ret = d;
	d->handle = ret;
	d->name = name;
	buildRefCountVector(d->occRefCounts);
	Devices[nextHandle++] = d;
	return  ret;
}
void Occupancy::buildRefCountVector(std::vector<std::vector<int>> &occRefCounts) {
	occ_num_range_segments = this->getRangeMax() / this->getRangeSegmentSize();
	occ_num_angle_segments = this->getAngleMax() / this->getAngleSegmentSize();
	occ_half_angles = occ_num_angle_segments / 2;
	//printf(" range max %d  range segment %d angle max %d angle segment %d rows: %d columns: %d half_angles: %d\n"
	//	, this->getRangeMax(), this->getRangeSegmentSize(), this->getAngleMax(), this->getAngleSegmentSize()
	//	, occ_num_range_segments, occ_num_angle_segments, occ_half_angles);

	occRefCounts.resize(occ_num_range_segments, std::vector<int>(occ_num_angle_segments, 0));

}


// setters


void Occupancy::setPUBClient(PUBClient *parm) {
	this->pub_client = parm;
}


void Occupancy::setRangeMax(int size)
{
	this->rangeMax = size;
}
void Occupancy::setRangeMin(int size)
{
	this->rangeMin = size;
}
void Occupancy::setAngleMax(int size)
{
	this->angleMax = size;
}

void Occupancy::setRangeSegmentSize(int size)
{
	this->rangeSegmentSize = size;
}
void Occupancy::setAngleSegmentSize(int size)
{
	this->angleSegmentSize = size;
}
void Occupancy::setCellExpired(int length) {
	this->cellExpired = length;
}
void Occupancy::setCellMax(int val) {
	this->cellMax = val;
}
void Occupancy::setMaxVelocity(string velocity) {
	this->maxVelocity = velocity;
}
// getters

int Occupancy::getRangeMax()
{
	return this->rangeMax;
}
int Occupancy::getRangeMin()
{
	return this->rangeMin;
}
int Occupancy::getAngleMax()
{
	return this->angleMax;
}

int Occupancy::getRangeSegmentSize()
{
	return this->rangeSegmentSize;
}
int Occupancy::getAngleSegmentSize()
{
	return this->angleSegmentSize;
}

int Occupancy::getCellExpired() {
	return this->cellExpired;
}
int Occupancy::getCellMax()
{
	return this->cellMax;
}

string Occupancy::getMaxVelocity() {
	return this->maxVelocity;
}

// q handling
bool Occupancy::addToQueue(occ_item_t *item) {
	if (this->sendq.size() >= max_q_size) {
		cout << "couldn't add to occ q" << endl;
		while (true) {
			occ_item_t * item = removeFromQueue();
			if (nullptr == item) break;
		}
		return false;
	
	}
	pthread_mutex_lock(&qlock);
	this->sendq.push(item);
	this->pub_client->setHeartOccQSize(max);
	this->pub_client->setHeartOccQMax(max_q_size);
	if (this->sendq.size() > max) {
		max = this->sendq.size();
		this->pub_client->setHeartOccQSize(max);
		this->pub_client->setHeartOccQMax(max_q_size);
	}
	Occupancy::available = true;
	pthread_mutex_unlock(&qlock);
	return true;
}
Occupancy::occ_item_t * Occupancy::removeFromQueue() {
	pthread_mutex_lock(&qlock);
	Occupancy::occ_item_t * item = nullptr;
	if (!this->sendq.empty()) {
		Occupancy::available = true;
		item = this->sendq.front();
		this->sendq.pop();
	}
	else {
		Occupancy::available = false;
	}
	pthread_mutex_unlock(&qlock);
	return item;
}
void Occupancy::setMaxQSize(int parm)
{
	max_q_size = parm;
}

void Occupancy::publish(OCC_HANDLE handle, PUBClient *pub_client
	, iSYSTargetList *targetList
	, string topic, string lastwill, float32_t adjx, float32_t adjy, float32_t adjAngle) {
	occ_item_t * item = new occ_item_t();
	item->pubclient = pub_client;
	item->targetList = new iSYSTargetList();
	memcpy(item->targetList, targetList, sizeof(iSYSTargetList));
	//item->topic.append(this->pub_client->getTopic());
	item->topic.append(topic);
	item->lastwill.append(lastwill);
	item->adjx = adjx;
	item->adjy = adjy;
	item->adjAngle = adjAngle;
	item->device = (occ_device *)handle;
	if (!addToQueue(item)) delete item;
}

void Occupancy::updateGridData(occ_device *dev) {
	if (!updating) {
		if (debugMsg) {
			cout << "\ndevice : " << dev->name << " handle: " << dev->handle;
		}
		for (int j = 0; j < occ_num_range_segments; j++) {
			if (debugMsg) {
				cout << endl;
			}
			for (int i = 0; i < occ_num_angle_segments; i++) {
				signed int cell = dev->occRefCounts[j][i];
				signed int cell_expiration = this->cellExpired;
				if (cell > 0) cell--;
				if (debugMsg) {
					cout << cell << "\t";
				}
				dev->occRefCounts[j][i] = cell;
			}
		}
		if (debugMsg) {
			cout << endl;
		}
	}
}

void Occupancy::clearGridData(occ_device *dev) {
	//occRefCounts.clear();
	for (int j = 0; j < occ_num_range_segments; j++) {
		for (int i = 0; i < occ_num_angle_segments; i++) {
			dev->occRefCounts[j][i] = 0;
		}
	}
}
////////////////

void Occupancy::updateGrid(occ_device *dev, iSYSTargetList *targetList) {
	for (int i = 0; i < targetList->nrOfTargets; i++)
	{
		bool angleNegative = false;
		float32_t v = targetList->targets[i].velocity;
		float32_t range = targetList->targets[i].range;
		float32_t angle = targetList->targets[i].angle;


		// no idea what these messages are but they offset the v 0.0 target for the same angle and range
		if ((v < 0.152 && v > 0.150) || (v > -0.152 && v < -0.15)) {
			v = 0.0;
			//continue;
		}

		if (debugMsg && range < 1.0) {
			printf("angle %f velocity %f range: %f\n"
				, angle, v, range);
		}

		if (angle < -0.001) {
			angleNegative = true;
		}

		float32_t absangle = fabs(angle);

		int rangemax = this->rangeMax;
		int anglemax = this->angleMax / 2;

		if (absangle > anglemax || range > rangemax) {
			continue;
		}

		signed int range_idx = static_cast<signed int>(range / this->rangeSegmentSize);
		signed int angle_idx = static_cast<signed int>(absangle / this->angleSegmentSize);

		if (angleNegative)
		{
			if (angle_idx > occ_half_angles) {
				printf("angle_idx: %d > occ_half_angles: %d\n"
					, angle_idx, occ_half_angles);
				continue;
			}
			angle_idx = occ_half_angles - angle_idx;
		}
		else {
			angle_idx = occ_half_angles + angle_idx;
		}

		if (angle_idx > occ_num_angle_segments) {
			printf("angle_idx: %d > occ_num_angle_segments: %d\n"
				, angle_idx, occ_num_angle_segments);
			continue;
		}
		if (range_idx > occ_num_range_segments) {
			printf("range_idx: %d > occ_num_range_segments: %d\n"
				, range_idx, occ_num_range_segments);
			continue;
		}
		if (range_idx > 0) {
			range_idx--;
		}
		else {
			printf("range_idx: %d !>  0 : %d\n"
				, range_idx, occ_num_range_segments);
			continue;
		}
		if (angle_idx > 0) {
			angle_idx--;
		}
		else {
			printf("angle: %f exceeds max angle: %d\n",angle, anglemax);
			printf("angle_idx: %d > occ_num_angle_segments: %d\n"
				, angle_idx, occ_num_angle_segments);
			continue;
		}
		signed int count = dev->occRefCounts[range_idx][angle_idx];

		if (v < .001 && v > -.001) { // considered no movement
			if (count < this->cellMax) {
				count++;
			}
			if (count > this->cellExpired) {
				float32_t velocity = targetList->targets[i].velocity = 0.0;
				float32_t range = targetList->targets[i].range = 0.0;
				float32_t angle = targetList->targets[i].angle = 0.0;
			}
		}
		else if (count < this->cellExpired && count > 0) {
			count--;
		}

		//cout << "v: " << to_string(velocity).c_str() << " range: "
		//	<< to_string(range).c_str() << " angle: " << to_string(angle).c_str()
		//	<< " range_idx: " << range_idx << " angle_idx:" << angle_idx << " count: " << count << endl;
		//Sleep(5);
		try {
			if (debugMsg && range_idx < 5) {
				//printf("range_idx: %d angle_idx %d\n"
				//	, range_idx, angle_idx);
			}
			dev->occRefCounts[range_idx][angle_idx] = count;
		}
		catch (exception e) {
			cout << "refcounts error " << e.what() << endl;
		}
	}
}

void Occupancy::formatTheList(iSYSTargetList *targetList, const char * description, string &sb, uint16_t &items) {
	bool firsttime = true;


	//sortRange(targetList->targets, targetList->nrOfTargets);

	for (int i = 0; i < targetList->nrOfTargets; i++) {
		char buff[128];

		if (!firsttime) {
			sb.append(",");
		}
		items++;
		firsttime = false;
		int size = snprintf(buff, sizeof(buff), "{ \"s\":%.1f ,\"r\":%.1f ,\"v\":%.2f ,\"a\":%.1f ,\"r\":%f ,\"r2\":%f ,\"ts\":%" PRIu64 "}\n"
			, targetList->targets[i].signal
			, targetList->targets[i].range
			, targetList->targets[i].velocity
			, targetList->targets[i].angle
			, targetList->targets[i].reserved1
			, targetList->targets[i].reserved2
			, targetList->targets[i].timestamp);
		sb.append(buff, size);

	}

}

void Occupancy::formatMQTTMessage(iSYSTargetList *targetList
	, string &sb
	, string &sb_nearest, uint16_t &items
	, float32_t xfloat, float32_t yfloat, float32_t anglefloat) {
	std::string::size_type sz;
	char buffx[16];
	char buffy[16];
	char buffangle[16];
	char buffrange[16];
	char buffvelocity[16];
	json j;
	json jn;
	json j_time;
	json j_object;
	json j_array;
	string stest;
	bool firsttime = true;
	time_t now;
	bool min_changed = false;
	if (targetList->nrOfTargets < 1) return;
	float min_range = 1000.0;
	int rmaxcnt = 0;
	int bscnt = 0;
	int expcnt = 0;
	float32_t vmax = stof(this->maxVelocity, &sz);
	float32_t rmax = (float)this->rangeMax;
	// cout << "format mqtt targets: " << targetList->nrOfTargets << endl;
	for (int i = 0; i < targetList->nrOfTargets; i++)
	{
		float32_t v = targetList->targets[i].velocity;
		float32_t r = targetList->targets[i].range;
		float32_t a = targetList->targets[i].angle;


		//
		// these are some type of errant messages
		//
		if ((v < 0.152 && v > 0.150) || (v > -0.152 && v < -0.15)) {
		//	bscnt++; continue;
			bscnt++;
			v = 0.0;
		}

		float32_t vabs = fabs(v);

		if (vabs < vmax && r < rmax)
		{

			if (r == 0.0) {
				expcnt++;
				continue; // this is set in updateGrid when a cell expires
			}
			if (firsttime) {
				time(&now);
				char buf[sizeof "2011-10-08T07:07:09Z"];
				strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
				j_time[JS_TIME] = buf;
			}
			items++;
			firsttime = false;
			try {

				double rad = (a + anglefloat)*(PI / 180);
				double xcoord = xfloat + (r * cos(rad));
				double ycoord = yfloat + (r * sin(rad));

				// format values for message
				memset(buffx, 0, sizeof(buffx));
				snprintf(buffx, sizeof(buffx), "%.1f", xcoord);
				memset(buffy, 0, sizeof(buffy));
				snprintf(buffy, sizeof(buffy), "%.1f", ycoord);
				memset(buffrange, 0, sizeof(buffrange));
				snprintf(buffrange, sizeof(buffrange), "%.1f", r);
				memset(buffangle, 0, sizeof(buffangle));
				snprintf(buffangle, sizeof(buffangle), "%.1f", a + anglefloat);
				memset(buffvelocity, 0, sizeof(buffvelocity));
				snprintf(buffvelocity, sizeof(buffvelocity), "%.1f", v);

				// build the message
				j_object[JS_MSG_RANGE] = buffrange;
				j_object[JS_MSG_ANGLE] = buffangle;
				j_object[JS_MSG_VELOCITY] = buffvelocity;
				j_object[JS_MSG_VELOCITY] = buffvelocity;
				j_object[JS_MSG_X] = buffx;
				j_object[JS_MSG_Y] = buffy;
				j_array.push_back(j_object);

				// if this is nearer than the last one
				if (r < min_range) {
					min_changed = true;
					j_nearest.clear();
					j_nearest.push_back(j_object);
					min_range = r;
				}
			}
			catch (exception e) {
				cout << e.what() << anglefloat << endl;
			}
		}
		else {
			//cout << "exceeded vmax " << v << " rmax " << r << endl;
			rmaxcnt++;
			continue;
		}
	}
//	cout << "items: " << items << " rmax : " << rmax 
//		<< "  vmax : " << vmax 
//		<< " rmaxcnt: " << rmaxcnt 
//		<< "  bs: " << bscnt  
//		<< " exp: " << expcnt
//		<< endl;
	if (!firsttime) {
		j.push_back(j_time);
		j.push_back(j_array);
		sb.append(j.dump());
		if (min_changed) {
			jn.push_back(j_time);
			jn.push_back(j_nearest);
			sb_nearest.append(jn.dump());
		}
	};
}

void Occupancy::notify(occ_item_t *item, char desc[], bool print) {
	string buff;
	string nearest;
	uint16_t items = 0;
	string topic = item->topic;
	string nearesttopic = item->topic;
	string lastwill = item->lastwill;

	// this was used to take snapshots of the target list after entering comments on the console
	//formatTheList(item->targetList, "this is a new one", buff, items);
	//if (print) {
		// printTheList("listprint.txt", targetList, "a", desc, buff);
	//}

	buff.clear();
	nearest.clear();

	formatMQTTMessage(item->targetList, buff, nearest, items, item->adjx, item->adjy, item->adjAngle);


	if (buff.size() > 0) {
		topic.append("/targets");
		this->pub_client->publish(topic, item->lastwill, (char *)buff.c_str());
	}
	if (nearest.size() > 0) {
		nearesttopic.append("/nearest");
		//this->pub_client->publish(nearesttopic, item->lastwill, (char *)nearest.c_str());
	}

}
