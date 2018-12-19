#pragma once
#include <string>
#include "json.hpp"
#include "serial_radarAPI_if.h"
#include "CivetServer.h"
#include "ftd2xx.h"
#include "LibFT4222.h"
#include "PUBClient.h"
#include "Occupancy.h"

using namespace nlohmann;
using namespace std;

#define PI 3.14159265
// JSON Elements
#define JS_COMPORT "COMport"
#define JS_PRODUCTINFO "ProductInfo"
#define JS_SERIALNUMBER "SerialNumber"
#define JS_DSP_HARDWAREVERSION "DspHardwareVersion"
#define JS_RFE_HARDWAREVERSION "RfeHardwareVersion"
#define JS_X "x"
#define JS_FREQUENCY "FrequencyChannel"
#define JS_Y "y"
#define JS_ANGLE "Angle"
#define JS_NEAR_RANGE_MARGIN "NearRangeMargin"
#define JS_MAIN_RANGE_MARGIN "MainRangeMargin"
#define JS_LONG_RANGE_MARGIN "LongRangeMargin"
#define JS_CLUSTERING "ClusteringEnable"
#define JS_RCSCALIBRATION "RCSCalibration"
#define JS_MQTT_BROKER "MQTTbroker"
#define JS_MQTT_PORT "MQTTport"
#define JS_MQTT_CLIENT "MQTTclient"
#define JS_MQTT_PASSWORD "MQTTpassword"
#define JS_MQTT_TOPIC "MQTTtopic"
#define JS_NOTIFY "Notify"
#define JS_RANGE_MAX "RangeMax"
#define JS_RANGE_MIN "RangeMin"
#define JS_RANGE_SEGMENT_SIZE "RangeSegmentSize"
#define JS_ANGLEMAX "AngleMax"
#define JS_ANGLE_SEGMENT_SIZE "AngleSegmentSize"
#define JS_CELL_EXPIRED "CellExpired"
#define JS_CELL_MAX "CellMax"
#define JS_MAX_VELOCITY "MaxVelocity"
#define JS_LIST_ENTRY "ListEntry"
#define JS_DEVICE_NAME "DeviceName"
#define JS_DEVICE_ADDRESS "DeviceAddress"

/*
JSON HeartBeat Messages
*/
#define JS_TIME "Time"
#define JS_STATUS "Status"
#define JS_MSG_OCCQ_MAX "occqmax"
#define JS_MSG_OCCQ_SIZE "occqsize"
#define JS_MSG_PUBSUBQ_MAX "pubsubmax"
#define JS_MSG_PUBSUBQ_SIZE "pubsubqsize"

/*
JSON Messages
*/
#define JS_MSG_RANGE "r"
#define JS_MSG_ANGLE "a"
#define JS_MSG_VELOCITY "v"
#define JS_MSG_X "x"
#define JS_MSG_Y "y"


#define FORM_START_ACQUISITION "start"
#define FORM_STOP_ACQUISITION "stop"

class Config
{
public:
	Config();
	Config(char * filename);
	virtual ~Config();
	Occupancy *occupancy;

private:
	int doesFileExist(const char *filename);
	void	fromMain(json j);
	char * filename;
	string mqttBroker;
	string mqttPort;
	string mqttClient;
	string mqttPassword;
	string mqttTopic;
	PUBClient *pub_client;
	string notify;
	bool populated;
	bool confile_corrupt;
	bool debugenabled;
	struct configentries 
	{
		string comPort;
		iSYSHandle_t pHandle;
		FT_HANDLE ftHandle;
		OCC_HANDLE occHandle;
		string SPISerialNumber;
		string ListEntry;
		string DeviceName;
		string DeviceAddress;
		string FrequencyChannel;
		string x;
		string y;
		string angle;
		string ThresholdMovingTargetsNearRangeMargin;
		string ThresholdMovingTargetsMainRangeMargin;
		string ThresholdMovingTargetsLongRangeMargin;

		string FirmwareVersion;
		string SerialNumber;
		string DspHardwareVersion;
		string RfeHardwareVersion;
		string ProductInfo;
		bool ProcessingRcsCalibrationEnable;
		bool TargetClusteringEnable;

	} entries;

	FILE *fconfig;

public:
	bool 		toJSON(json &j);
	bool		fromJSON(json &j);
	bool		populate(char  *filename);
	bool		isPopulated();
	bool		isConfFileCorrupt();
	bool		isDebugEnabled();
	string		getcomPort();
	iSYSHandle_t getHandle();
	FT_HANDLE   getFtHandle();
	string		getSPISerialNumber();
	string		getmqttBroker();
	string		getmqttPort();
	string		getmqttClient();
	string		getmqttPassword();
	string		getmqttTopic();
	PUBClient * getPUBClient();
	OCC_HANDLE getOCCHandle();
	string		getListEntry();
	string		getDeviceName();
	string		getDeviceAddress();
	string		getFrequencyChannel();
	string		getx();
	string		gety();
	string		getAngle();
	string		getThresholdMovingTargetsNearRangeMargin();
	string		getThresholdMovingTargetsMainRangeMargin();
	string		getThresholdMovingTargetsLongRangeMargin();
	string		getFirmwareVersion();
	string		getSerialNumber();
	string		getDspHardwareVersion();
	string		getRfeHardwareVersion();
	string		getProductInfo();
	string		getNotify();
	bool		isTargetClusteringEnable();
	bool		isProcessingRcsCalibrationEnable();

	void		setcomPort(string parm);
	void		setHandle(iSYSHandle_t parm);
	void		setFtHandle(FT_HANDLE parm);
	void		setSPISerialNumber(string parm);
	void		setmqttBroker(string  parm);
	void		setmqttPort(string parm);
	void		setmqttClient(string  parm);
	void		setmqttPassword(string  parm);
	void		setmqttTopic(string  parm);
	void		setPUBClient(PUBClient *parm);
	void		setOccupancy(Occupancy *parm);
	void		setOCCHandle(OCC_HANDLE parm);
	void		setListEntry(string parm);
	void		setDeviceName(string parm);
	void 		setDeviceAddress(string parm);
	void 		setFrequencyChannel(string  parm);
	void		setx(string parm);
	void 		sety(string parm);
	void 		setAngle(string  parm);

	void 		setThresholdMovingTargetsNearRangeMargin(string  parm);
	void 		setThresholdMovingTargetsMainRangeMargin(string  parm);
	void 		setThresholdMovingTargetsLongRangeMargin(string  parm);
	void 		setFirmwareVersion(string  parm);
	void 		setSerialNumber(string  parm);
	void 		setDspHardwareVersion(string  parm);
	void 		setRfeHardwareVersion(string  parm);
	void 		setProductInfo(string  parm);
	void		setNotify(string parm);
	void		setProcessingRcsCalibrationEnable(bool parm);
	void		setTargetClusteringEnable(bool parm);
	void		createDefault(char *filename);
	void		webFormPrint(string &pretty);
	bool		webFormProcess(struct mg_connection *conn, string &s);
	bool		webAcquisitionProcess(struct mg_connection *conn, string &s);
};