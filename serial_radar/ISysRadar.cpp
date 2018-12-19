
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <list>
#include <stdlib.h>
#include <signal.h>
#include <functional>
#include <conio.h>
#include <chrono>
#include <inttypes.h>
#include "serial_radarAPI_if.h"
#include "ISysRadar.h"
#include "ISysSPI.h"

using namespace std;
using namespace nlohmann;

ISysRadar::ISysRadar()
{
}

ISysRadar::~ISysRadar()
{
}

#define MAXPORTS 15

int ISysRadar::scanPorts(Config *cfgs[], int max)
{
	iSYSHandle_t pHandle;
	iSYSResult res = ERR_OK;
	int entries = 0;
	for (int comport = 0; comport < MAXPORTS && max > entries; comport++) {
		//cout << "\nScanning COM port.. " << comport << "\n";
		res = iSYS_initComPort(&pHandle, static_cast<uint8_t>(comport), ISYS_BAUDRATE_115200);
		if (res != ERR_OK) {
			//cout << "error: " << ERROR_CODES[res] << "\n";
			Sleep(500);
		}
		else {
			cout << "COM port " << comport << " Initialized " << " handle " << pHandle << endl;

			Config * cfgx = new Config();
			cfgx->setcomPort(to_string(comport));
			cfgx->setHandle(pHandle);
			cfgx->setx("0.0");
			cfgx->sety("0.0");
			cfgx->setAngle("0.0");
			cfgs[entries++] = cfgx;
		}
	}
	return entries;
}

iSYSResult ISysRadar::attachSPIInterface()
{
	int retries = 0;
	stopAcquisition();

	Sleep(1000);

	while (retries++ < DISCOVERY_RETRY) {

		ISysSPI *spi = ISysSPI::scanAlive();

		if (nullptr == spi) {

			cout << "didn't receive anything from any of the SPI interfaces" << endl;
			res = ERR_HANDLE_NOT_INITIALIZED;

		}
		else {
			cout << "\n==================== Found the SPI " << spi->ftHandle << "  handle: " << this->pHandle << " COM " << this->cfg->getcomPort().c_str() << endl;

			this->spi = spi;
			this->cfg->setFtHandle(spi->ftHandle);
			res = ERR_OK;
			break;
		}

	}


	startAcquisition();

	return res;
}

iSYSResult ISysRadar::init()
{

	this->pubclient = this->cfg->getPUBClient();
	// use the handle created when the port was discovered
	this->pHandle = this->cfg->getHandle();

	// let's get it set up
	res = initRadar();


	return res;
}

bool smartCopy(iSYSTargetList *buildList, iSYSTargetList * targetList)

{

	for (int i = 0; i < targetList->nrOfTargets; i++) {
		bool match = false;
		for (int j = 0; j < buildList->nrOfTargets; j++) {
			if (targetList->targets[i].angle == buildList->targets[j].angle
				&& targetList->targets[i].range == buildList->targets[j].range
				&& targetList->targets[i].velocity == buildList->targets[j].velocity) {
				match = true;
				//cout << "match r: " << targetList->targets[i].range << " v: " << targetList->targets[i].velocity << " a:" << targetList->targets[i].angle << endl;
				break;
			}
		}
		// removing targets that matched caused problems with v 0.0 messages
		// for other use cases this may be useful so it's not deleted
		//if (!match) {
		if (buildList->nrOfTargets >= MAX_TARGETS) {
			cout << "build list targets > MAX_TARGETS " << buildList->nrOfTargets << endl;
			return false;
		}
		uint16_t loc = buildList->nrOfTargets;
		buildList->targets[loc].angle = targetList->targets[i].angle;
		buildList->targets[loc].range = targetList->targets[i].range;
		buildList->targets[loc].velocity = targetList->targets[i].velocity;
		buildList->targets[loc].x = targetList->targets[i].x;
		buildList->targets[loc].y = targetList->targets[i].y;
		buildList->nrOfTargets++;
		//}
	}
	return true;
}

void * ISysRadar::run(void  *pbuf)
{
	// let's get going

	res = stopAcquisition();
	if (res != ERR_OK) {
		return nullptr;
	}

	res = startAcquisition();
	vector<unsigned char> msgBuf;
	int notify_interval = 0;

	pthread_mutex_init(&buildlock, nullptr);
	iSYSTargetList *buildList = new iSYSTargetList();
	while (true) {
		std::string::size_type sz;
		int notifyCycle = std::stoi(main_cfg->getNotify(), &sz);
		if (nullptr == this->spi) break;
		if (this->spi->spiCheck(msgBuf, this->spi->ftHandle)) {
			uint16_t rcvdSize = (uint16_t)msgBuf.size();
			if (rcvdSize > 0) {
				unsigned char *char_ptr = nullptr;
				uint32_t *pRead_data = (uint32_t *)msgBuf.data();
				char_ptr = (unsigned char *)pRead_data;

				// create a target list

				if (nullptr != targetList) {
					delete targetList;
					targetList = nullptr;
				}
				targetList = new iSYSTargetList();
				iSYSResult res = decodeMessage(char_ptr, rcvdSize, targetList);
				if (ERR_OK != res) {
					fprintf(stderr, "ERR from SPI %s\n", ERROR_CODES[res]);
					printHex(char_ptr, rcvdSize);
					msgBuf.clear();
					// stop and start the radar unit
					res = stopAndStart();
				}

				pthread_mutex_lock(&buildlock);
				smartCopy(buildList, targetList);
				pthread_mutex_unlock(&buildlock);

			}
			msgBuf.clear();
		}
		if (notify_interval++ > notifyCycle && nullptr != buildList && buildList->nrOfTargets > 0)
		{
			// publish but don't save it to the file
			// cout << "build targets: " << buildList->nrOfTargets << endl;
			notify(buildList, "automated\n", false);
			buildList->nrOfTargets = 0;
			notify_interval = 0;
		}

		Sleep(50);
	}
	return nullptr;
}

iSYSResult ISysRadar::start()
{
	this->spi->start(this->buft);

	return ERR_OK;
}
iSYSResult ISysRadar::stop()
{

	return ERR_OK;
}

iSYSResult ISysRadar::startAcquisition()
{
	iSYSResult res;
	/* start measurement */

	res = iSYS_StartAcquisition(this->pHandle, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not start measurement!\n";
	}
	Sleep(500);
	return res;
}
iSYSResult ISysRadar::stopAcquisition()
{
	iSYSResult res;

	res = iSYS_StopAcquisition(this->pHandle, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not stop measurement! \n";
	}

	/* NOTE start and stop measurement may take up to 500ms! */
	//cout << "wait for 500ms.. \n";
	Sleep(500);

	return res;
}
iSYSResult ISysRadar::exitSystem()
{
	iSYSResult res;
	//cout << "stop measurement.. \n";
	res = iSYS_exitSystem(this->pHandle, this->deviceAddress);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not exit system! \n";
	}
	return res;
}
void ISysRadar::setConfig(Config *cfg) {
	this->cfg = cfg;
}

Config *ISysRadar::getConfig() {
	return this->cfg;
}

void ISysRadar::setPhandle(iSYSHandle *pHandle) {
	this->pHandle = pHandle;
}
iSYSHandle *ISysRadar::getPhandle() {
	return this->pHandle;
}

bool ISysRadar::spiCheck(vector<unsigned char> &) {
	return true;
}


void ISysRadar::intHandler(int dummy = 0) {
	printf("good bye\n");
	running = false;
}

iSYSResult_t ISysRadar::getRadarConfig(bool update) {
	float32_t version;
	uint16_t major, fix, minor;
	uint16_t productInfo;
	sint16_t margin = 0;
	char buf[32];
	json j;

	uint32_t serialNumber;
	char deviceName[21];
	this->deviceAddress = 100;
	res = iSYS_getApiVersion(&version);

	//cout << "\n    serial radarAPI Version: ---- " << version << "\n";


	res = iSYS_getThresholdMovingTargetsNearRangeMargin(this->pHandle, ISYS_LOCATION_RAM, &margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read NearRangeMargin\n";
	}
	else {
		memset(buf, 0, sizeof buf);
		snprintf(buf, sizeof buf, "%d", margin);
		sensorConfig.append("NearRangeMargin: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	this->cfg->setThresholdMovingTargetsNearRangeMargin(buf);


	res = iSYS_getThresholdMovingTargetsMainRangeMargin(this->pHandle, ISYS_LOCATION_RAM, &margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read MainRangeMargin\n";
	}
	else {
		memset(buf, 0, sizeof buf);
		snprintf(buf, sizeof buf, "%d", margin);
		sensorConfig.append("MainRangeMargin: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	this->cfg->setThresholdMovingTargetsMainRangeMargin(buf);

	res = iSYS_getThresholdMovingTargetsLongRangeMargin(this->pHandle, ISYS_LOCATION_RAM, &margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read LongRangeMargin\n";
	}
	else {
		memset(buf, 0, sizeof buf);
		snprintf(buf, sizeof buf, "%d", margin);
		sensorConfig.append("LongRangeMargin: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	this->cfg->setThresholdMovingTargetsLongRangeMargin(buf);


	// display some info

	/* get modulation type */

	res = iSYS_getFrequencyChannel(this->pHandle, &radarchannel, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read modulation type! \n";
	}
	else {

		if (radarchannel == ISYS_CHANNEL_1) {
			sensorConfig.append("up-ramp active channel ");
		}
		else {
			sensorConfig.append("down-ramp active channel ");
		}
		sensorConfig.append(to_string(radarchannel));
		sensorConfig.append("\n");
	}
	memset(buf, 0, sizeof buf);
	snprintf(buf, sizeof buf, "%d", radarchannel);

	this->cfg->setFrequencyChannel(buf);

	//* get RCS output setting from RAM 
	res = iSYS_getRcsOutputEnable(this->pHandle, ISYS_LOCATION_RAM, &enable, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read RCS setting! \n";
	}
	else {
		if (enable == 0) {
			this->cfg->setProcessingRcsCalibrationEnable(false);
			sensorConfig.append("RCS output OFF \n");
		}
		else {
			this->cfg->setProcessingRcsCalibrationEnable(true);
			sensorConfig.append("RCS output ON \n");
		}
	}


	//* get clustering from RAM 

	res = iSYS_getTargetClusteringEnable(this->pHandle, ISYS_LOCATION_RAM, &enable, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read clustering setting! \n";
	}
	else {
		if (enable == 0) {
			sensorConfig.append("clustering OFF \n");
			this->cfg->setTargetClusteringEnable(false);
		}
		else {
			sensorConfig.append("clustering ON \n");
			this->cfg->setTargetClusteringEnable(true);

		}
	}

	/* get devicename */
	res = iSYS_ReadDeviceName(this->pHandle, deviceName, 21, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read devicename\n";
	}
	else {
		sensorConfig.append("device name: ");
		sensorConfig.append(deviceName);
		sensorConfig.append("\n");
	}
	this->cfg->setDeviceName((char *)deviceName);

	/* get device address */

	res = iSYS_getDeviceAddress(this->pHandle, &addr, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read address \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d", this->deviceAddress);
		sensorConfig.append("device address: ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	this->cfg->setDeviceAddress(buf);

	/* get firmware version */

	res = iSYS_getFirmwareVersion(this->pHandle, &major, &fix, &minor, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read firmware version\n";
	}
	else {
		// cout << "firmware version: " << major << "." << minor << "\n";
	}
	//this->cfg->setFirmwareVersion(major << "." << minor);
	//j["FirmwareVersion"] = this->cfg->getFirmwareVersion();

	/* get product info */

	res = iSYS_getProductInfo(this->pHandle, &productInfo, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read product info! \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d", productInfo);
		sensorConfig.append("product info:  ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	this->cfg->setProductInfo(buf);

	/* get serialnumber */

	res = iSYS_getSerialNumber(this->pHandle, &serialNumber, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read serial number \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d", serialNumber);
		sensorConfig.append("serial number:  ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");

	}
	this->cfg->setSerialNumber(buf);

	/* get DSP hw version from sensor */

	res = iSYS_getDspHardwareVersion(this->pHandle, &major, &fix, &minor, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read DSP version! \n";
	}
	else {
		snprintf(buf, sizeof buf, "%d:%d", major, minor);
		sensorConfig.append("DSP version:   ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	this->cfg->setDspHardwareVersion(buf);

	/* get RFE hw version from sensor */

	res = iSYS_getRfeHardwareVersion(this->pHandle, &major, &fix, &minor, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not read RFE version! \n\n";
	}
	else {
		snprintf(buf, sizeof buf, "%d:%d", major, minor);
		sensorConfig.append("RFE version:   ");
		sensorConfig.append(buf);
		sensorConfig.append("\n");
	}
	this->cfg->setRfeHardwareVersion(buf);

	if (res == ERR_OK) {
		json j1;
		this->cfg->toJSON(j1);
		string print = j1.dump(4);
	}
	//fprintf(fp, print.c_str());
	// cout << print.c_str() << endl;
	return res;
}
int ISysRadar::doesFileExist(const char *filename) {
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}

iSYSResult_t ISysRadar::initRadar()
{
	/* variables */
	iSYSResult_t res;

	this->deviceAddress = 100;

	/******************************************/
	/** set and read iSYS-5010 configuration **/
	/******************************************/

	res = startSensorConfig();

	if (res != ERR_OK) {
		cout << "error: Can't start the sensor --  " << ERROR_CODES[res] << "\n";
		return res;
	}
	Sleep(500);



	// cout << "Radar successfully initialized\n\n";

	//cout << sensorConfig.c_str();
	printTheList("listprint.txt", nullptr, "w", "Sensor:\n", sensorConfig);

	return res;
}

iSYSResult ISysRadar::startSensorConfig()
{
	/* init system with deviceaddress and timeout 1000ms */
	cout << "initializing sensor.. \n";
	res = iSYS_initSystem(this->pHandle, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "could not initialize system -- program closes automatically";
		Sleep(5000);
		return ERR_HANDLE_NOT_INITIALIZED;
	}

	radarchannel = (iSYSFrequencyChannel_t)stoi(this->cfg->getFrequencyChannel());
	res = iSYS_setFrequencyChannel(this->pHandle, radarchannel, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set up-ramp modulation! \n";
		return res;
	}
	/* set RCS output in target list ON and store the value in RAM */

	enable = this->cfg->isProcessingRcsCalibrationEnable() == true ? 1 : 0;

	res = iSYS_setRcsOutputEnable(this->pHandle, ISYS_LOCATION_RAM, enable, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set RCS output ON! \n";
	}

	/* set clustering enable and store value in RAM */

	enable = this->cfg->isTargetClusteringEnable() == true ? 1 : 0;

	res = iSYS_setTargetClusteringEnable(this->pHandle, ISYS_LOCATION_RAM, enable, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set clustering! \n";
	}

	//sint16_t margin = 20;
	sint16_t margin = stoi(this->cfg->getThresholdMovingTargetsNearRangeMargin());
	res = iSYS_setThresholdMovingTargetsNearRangeMargin(this->pHandle, ISYS_LOCATION_RAM, margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set NearRangeMargin! \n";
		return res;
	}
	margin = stoi(this->cfg->getThresholdMovingTargetsMainRangeMargin());
	res = iSYS_setThresholdMovingTargetsMainRangeMargin(this->pHandle, ISYS_LOCATION_RAM, margin, 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set MainRangeMargin! \n";
		return res;
	}

	margin = stoi(this->cfg->getThresholdMovingTargetsLongRangeMargin());
	res = iSYS_setThresholdMovingTargetsLongRangeMargin(this->pHandle, ISYS_LOCATION_RAM, margin, 100, 1000);

	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not set LongRangeMargin! \n";
		return res;

	}
	return res;
}


/****************************************************************************
*
*	Stop the acquisition and start it again
*
****************************************************************************/

iSYSResult ISysRadar::stopAndStart() {
	/* stop measurement */
	cout << "stop measurement.. \n";
	res = iSYS_StopAcquisition(this->pHandle, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not stop measurement! \n";
	}

	/* NOTE start and stop measurement may take up to 500ms! */
	cout << "wait for 500ms.. \n";
	Sleep(500);

	/* start measurement */
	cout << "start measurement.. \n";
	res = iSYS_StartAcquisition(this->pHandle, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not start measurement!\n";
	}
	return res;
}

void ISysRadar::printTheList(char *filename, iSYSTargetList *targetList, char *mode, char *description, string data) {
	FILE *fp = fopen(filename, mode);
	if (nullptr == targetList) {
		fprintf(fp, "%s", data.c_str());
	}
	else {
		fprintf(fp, "\ndescription %s\n\n", description);
		fprintf(fp, "%s\n", data.c_str());
		//sortRange(targetList->targets, targetList->nrOfTargets);
	}
	// sortRange(targetList);
	fclose(fp);
}

void ISysRadar::notify(iSYSTargetList_t *targetList, char desc[], bool print) {
	string buff;
	uint16_t items = 0;
	//formatTheList(targetList, "this is a new one", buff, items);
	if (print) {
		printTheList("listprint.txt", targetList, "a", desc, buff);
	}

	buff.clear();

	string topic = "/";
	topic.append(cfg->getSerialNumber());
	string lastwill = cfg->getmqttTopic();
	lastwill.append("/lastwill");
	if (nullptr != targetList) {
		std::string::size_type sz;   // alias of size_t
		// cout << "notify targets: " << targetList->nrOfTargets << endl;
		main_cfg->occupancy->publish(this->cfg->getOCCHandle(), this->pubclient, targetList, topic, lastwill
			, stof(this->cfg->getx(), &sz), stof(this->cfg->gety(), &sz), stof(this->cfg->getAngle(), &sz));
	}
}


/************************************************************
*
*	Shutdown the radar unit by stopping the acquisition
*	and exit the system
*
*************************************************************
*/
iSYSResult ISysRadar::closeRadar() {
	/* stop measurement */
	cout << "stop measurement.. \n";
	res = iSYS_StopAcquisition(this->pHandle, this->deviceAddress, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not stop measurement! \n\n";
	}

	/* NOTE start and stop measurement may take up to 500ms! */
	cout << "wait for 500ms.. \n";
	Sleep(500);
	/* exit system with this->deviceAddress */
	res = iSYS_exitSystem(this->pHandle, this->deviceAddress);
	if (res != ERR_OK) {
		system("cls");
		cout << "Error: " << ERROR_CODES[res] << "\n";
		cout << "could not exit system";
	}

	/* Close serialport connection */
	res = iSYS_exitComPort(this->pHandle);
	if (res == ERR_OK)
	{
		cout << "\n\n---------------------------------";
		cout << "\n  handle finished";
		cout << "\n---------------------------------";
	}
	else
	{
		system("cls");
		cout << "Error: " << ERROR_CODES[res] << "\n";
		cout << "could not exit system";
	}

	return res;
}

/********************************************************************************
*
*	Checks for the start sequence [0xFEED5010] in a target list message
*
**********************************************************************************/
bool ISysRadar::list_start(uint8_t msg_check[], uint8_t &entries)
{
	if (msg_check[0] == 0xFE && msg_check[1] == 0xED && msg_check[2] == 0x50 && msg_check[3] == 0x10) {
		entries = msg_check[7];
		return true;
	}
	return false;
}

/************************************************************************
*
*	Checks for the end sequence [0xFEED0000] in a target list message
*
*************************************************************************/
bool ISysRadar::list_end(uint8_t msg_check[]) {
	if (msg_check[0] == 0xFE && msg_check[1] == 0xED && msg_check[2] == 0x00 && msg_check[3] == 0x00) return true;
	return false;
}

/***********************************************************************
Function: decodes Message received from iSYS device.
Input arguments:
	Frame array:  array with from iSYS received target list message
	rcvdSize: number of bytes in the frame array
	targetList: struct with decoded target list
Return value:
	ErrorCode

***********************************************************************/
iSYSResult ISysRadar::decodeMessage(unsigned char * msg, uint16_t msgSize, iSYSTargetList_t *targetList)
{
	uint8_t * frames_start = nullptr;
	uint16_t frames_length = 0;
	uint8_t listEntries = 0;
	uint8_t msgState = LIST_BEGIN;

	for (int i = 0; i < msgSize; i++)
	{
		switch (msgState) {
		case LIST_BEGIN:
		{
			if (msgSize > 3)
			{
				if (!list_start((uint8_t *)(msg + i), listEntries))
				{
					printf("no idea what's coming in\n");
					msgState = LIST_ERROR;
					// initRadar();
					stopAndStart();
				}
				else
				{
					msgState = LIST_PROCESSING;
					i += 7;
					frames_start = (uint8_t *)(msg + i + 1);
					frames_length = 0;
				}
			}
			else
			{
				fprintf(stderr, "List buffer received %d bytes - must be 3 or greater\n", msgSize);
				msgState = LIST_ERROR;
			}
			break;
		}
		case LIST_PROCESSING:
		{
			if (!list_end((uint8_t *)(msg + i))) {
				i += 23;
				frames_length += 24;
			}
			else {
				// end of frame
				if (frames_start == nullptr) {
					printf("never set the frames_start pointer can't decode rcvd: %d i:%d frames_length:%d\n", msgSize, i, frames_length);
					//printHex(msg, i);
					msgState = LIST_ERROR;
					break;
				}
				// populate the target list
				iSYSResult res = decodeTargetFrame(frames_start
					, frames_length
					, 5010
					, 32
					, listEntries
					, targetList);
				if (ERR_OK != res)
				{
					printf("error from decode target %s \n", ERROR_CODES[res]);
					msgState = LIST_ERROR;
					break;
				}
				// process any additional
				msgState = LIST_BEGIN;
				i += 7;
			}
			break;
		}
		case LIST_ERROR:
			msgState = LIST_BEGIN;
			printf("received list error msgSize: %d i:%d frames_length:%d\n", msgSize, i, frames_length);
			return ERR_COMMAND_NO_VALID_FRAME_FOUND;
		}
	}
	return ERR_OK;
}


/***********************************************************************
Function: decodes target List frame received from iSYS device.
Input arguments:
	Frame array:  array with from iSYS received target list frame
	nrOfElements: number of bytes in the frame array
	productcode: product code of the connected iSYS (e.g. 6003, 4001, …)
	bitrate: resolution of the target list in the frame array (16-Bit or 32-Bit)
	argetList: struct for decoded target list Output arguments:
	targetList: struct with decoded target list
Return value:
	ErrorCode

***********************************************************************/
iSYSResult_t ISysRadar::decodeTargetFrame(uint8_t *pData
	, uint16_t nrOfElements
	, uint16_t productcode
	, uint8_t bitrate
	, uint8_t nrOfTargets
	, iSYSTargetList_t *targetList)
{
	uint8_t output_number = 0;

	if (nullptr == pData || nullptr == targetList)
	{
		return ERR_COMMAND_NO_VALID_FRAME_FOUND;
	}
	/* check for valid amount of targets */
	if ((nrOfTargets > MAX_TARGETS) && (nrOfTargets != 0xff))
	{
		printf("too many targets %d  max: %d\n", nrOfTargets, MAX_TARGETS);
		return ERR_COMMAND_MAX_DATA_OVERFLOW;
	}
	// printHex(pData, nrOfElements);
	targetList->nrOfTargets = nrOfTargets;
	targetList->clippingFlag = 0;
	targetList->outputNumber = output_number;
	if (nrOfTargets != 0xff) { //0xff clipping
		int i = 0;
		for (int j = 0; j < nrOfTargets; j++)
		{
			targetList->targets[j].signal = unpackFloat(pData + i, &i);
			targetList->targets[j].range = unpackFloat(pData + i, &i);
			targetList->targets[j].velocity = unpackFloat(pData + i, &i);
			targetList->targets[j].angle = unpackFloat(pData + i, &i);
			targetList->targets[j].reserved1 = unpackFloat(pData + i, &i);
			targetList->targets[j].reserved2 = unpackFloat(pData + i, &i);
			chrono::high_resolution_clock m_clock;

			targetList->targets[j].timestamp = chrono::duration_cast<chrono::milliseconds>
				(m_clock.now().time_since_epoch()).count();
		}
	}

	if (nrOfTargets == MAX_TARGETS) {
		targetList->error.iSYSTargetListError = TARGET_LIST_FULL;
	}
	else {
		targetList->error.iSYSTargetListError = TARGET_LIST_OK;
	}
	return ERR_OK;
}


/******************************************************************
*
*
*	functions for using the target list
*
*
*******************************************************************/
#define float32_t float

float ISysRadar::unpackFloat(const void *buf, int *i) {
	const unsigned char *b = (const unsigned char *)buf;

	uint32_t temp = 0;
	*i += 4;
	temp = ((b[0]) << 24 |
		(b[1] << 16) |
		(b[2] << 8) |
		b[3]);
	float f = *((float *)&temp);
	return f;
}


void ISysRadar::printHex(uint8_t *buf, uint16_t size)
{
	int j = 0;

	cout << "buff size " << size << endl;
	for (int i = 0; i < 16; i++)
		//	for (int i = 0; i < size; i++)
	{
		if (j == 0) printf("%d -- ", i);
		printf("%02X ", *(buf + i));
		if (j++ > 6) {
			printf("\n");
			j = 0;
		}
	}
	printf("\n\n");
}

void sortRange(iSYSTarget_t *targetList, int number);


bool ISysRadar::myfunction(int i, int j) { return (i < j); }

void ISysRadar::sortRange(iSYSTarget_t *targets, int number) {

	//std::vector<iSYSTarget_t> mytargets(targetList->targets, targetList->nrOfTargets);
	int x = sizeof(float);
	float32_t *array = (float32_t *)calloc(number, x);

	for (int i = 0; i < number; i++) {
		*(i + array) = targets[i].range;
	}
	vector<float32_t> vect;
	for (int i = 0; i < number; i++) {
		vect.push_back(*(array + i));
	}

	// using default comparison (operator <):
	std::sort(vect.begin(), vect.begin() + number);           //(12 32 45 71)26 80 53 33

	// print out content:
	std::cout << "myvector contains:";
	for (std::vector<float>::iterator it = vect.begin(); it != vect.end(); ++it)
		std::cout << ' ' << *it;
	std::cout << '\n';
	// using function as comp
//	std::sort(myvector.begin() + 4, myvector.end(), myfunction); // 12 32 45 71(26 33 53 80)
	free(array);
	// using object as comp
//	std::sort(myvector.begin(), myvector.end(), myobject);     //(12 26 32 33 45 53 71 80)
}

iSYSResult ISysRadar::processMessage(vector<unsigned char> msgBuf) {


	uint16_t rcvdSize = (uint16_t)msgBuf.size();
	unsigned char *char_ptr = nullptr;
	uint32_t *pRead_data = (uint32_t *)msgBuf.data();
	char_ptr = (unsigned char *)pRead_data;
	if (nullptr == targetList)	targetList = new iSYSTargetList();

	// create a target list
	iSYSResult res = decodeMessage(char_ptr, rcvdSize, targetList);
	if (ERR_OK != res) {
		fprintf(stderr, "ERR from SPI %s\n", ERROR_CODES[res]);
		printHex(char_ptr, rcvdSize);
		msgBuf.clear();

		//spiClose();

		//spiInit();

		// stop and start the radar unit
		res = stopAndStart();

	}

	return res;
}
