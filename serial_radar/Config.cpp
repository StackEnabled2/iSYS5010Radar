#include "Config.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include "pthreaddefs.h"
#include "ISysRadar.h"

Config::Config() {}

Config::Config(char * filename)
{
	if (doesFileExist(filename)) {
		if (populate(filename)) {
			populated = true;
		}
	}
	else {
		populated = false;
		printf("file %s didn't exist", filename);
		confile_corrupt = false;
	}
}


Config::~Config()
{

}

void Config::createDefault(char *filename) {
	populated = false;
	cout << "create default configuration" << endl;
	fconfig = fopen(filename, "w");
	json j1;
	this->toJSON(j1);
	string print = j1.dump(4);
	fprintf(fconfig, print.c_str());
	//cout << print.c_str() << endl;
	fclose(fconfig);

}

int Config::doesFileExist(const char *filename) {
	struct stat st;
	int result = stat(filename, &st);
	return result == 0;
}
bool	Config::isPopulated() {
	return populated;
}
/*
	Getters
*/

string		Config::getmqttBroker() {
	return mqttBroker;
}
string	Config::getmqttPort() {
	return mqttPort;
}
string		Config::getmqttClient() {
	return mqttClient;
}

string		Config::getmqttPassword() {
	return mqttPassword;

}
string		Config::getmqttTopic() {
	return mqttTopic;
}

PUBClient * Config::getPUBClient() {
	return pub_client;
}
string		Config::getcomPort() {
	return entries.comPort;
}
iSYSHandle_t Config::getHandle() {
	return entries.pHandle;
}
FT_HANDLE Config::getFtHandle() {
	return entries.ftHandle;
}
string Config::getSPISerialNumber() {
	return entries.SPISerialNumber;
}
string Config::getListEntry() {
	return entries.ListEntry;
}
string Config::getDeviceName() {
	return entries.DeviceName;
}
string 		Config::getDeviceAddress() {
	return entries.DeviceAddress;
}
string 	Config::getFrequencyChannel() {
	return entries.FrequencyChannel;
}
string Config::getx() {
	return entries.x;
}
string 		Config::gety() {
	return entries.y;
}
string 		Config::getAngle() {
	return entries.angle;
}
OCC_HANDLE Config::getOCCHandle() {
	return entries.occHandle;
}
string 		Config::getThresholdMovingTargetsNearRangeMargin() {
	return entries.ThresholdMovingTargetsNearRangeMargin;
}
string 		Config::getThresholdMovingTargetsMainRangeMargin() {
	return entries.ThresholdMovingTargetsMainRangeMargin;
}
string 		Config::getThresholdMovingTargetsLongRangeMargin() {
	return entries.ThresholdMovingTargetsLongRangeMargin;
}

string 		Config::getFirmwareVersion() {
	return entries.FirmwareVersion;
}
string 		Config::getSerialNumber() {
	return entries.SerialNumber;
}
string 		Config::getDspHardwareVersion() {
	return entries.DspHardwareVersion;
}
string 		Config::getRfeHardwareVersion() {
	return entries.RfeHardwareVersion;
}
string 		Config::getProductInfo() {
	return entries.ProductInfo;
}
string		Config::getNotify() {
	return notify;
};

bool		Config::isTargetClusteringEnable() {
	return entries.TargetClusteringEnable;
}
bool		Config::isProcessingRcsCalibrationEnable() {
	return entries.ProcessingRcsCalibrationEnable;
}


/*
		Setters
*/

void		Config::setmqttBroker(string  parm) {
	mqttBroker = parm;
}
void		Config::setmqttPort(string p) {
	mqttPort = p;

}
void		Config::setNotify(string p) {
	notify = p;
}
void		Config::setmqttClient(string  parm) {
	mqttClient = parm;
}
void		Config::setmqttPassword(string  parm) {
	mqttPassword = parm;
}
void		Config::setmqttTopic(string  parm) {
	mqttTopic = parm;
}
void Config::setPUBClient(PUBClient * parm) {
	pub_client = parm;
}
void Config::setOccupancy(Occupancy * parm) {
	occupancy = parm;
}
void Config::setOCCHandle(OCC_HANDLE  parm) {
	entries.occHandle = parm;
}
void		Config::setcomPort(string  parm) {
	entries.comPort = parm;
}
void		Config::setHandle(iSYSHandle_t parm) {
	entries.pHandle = parm;
}
void		Config::setFtHandle(FT_HANDLE parm) {
	entries.ftHandle = parm;
}
void		Config::setSPISerialNumber(string parm) {
	entries.SPISerialNumber = parm;
}
void		Config::setListEntry(string parm) {
	entries.ListEntry = parm;
}
void		Config::setDeviceName(string parm) {
	entries.DeviceName = parm;
}
void 		Config::setDeviceAddress(string parm) {
	entries.DeviceAddress = parm;
}
void		Config::setFrequencyChannel(string  parm) {
	entries.FrequencyChannel = parm;
};
void		Config::setx(string parm) {
	entries.x = parm;
}
void 		Config::sety(string parm) {
	entries.y = parm;
}
void		Config::setAngle(string  parm) {
	entries.angle = parm;
};
void		Config::setThresholdMovingTargetsNearRangeMargin(string  parm) {
	entries.ThresholdMovingTargetsNearRangeMargin = parm;
};
void		Config::setThresholdMovingTargetsMainRangeMargin(string  parm) {
	entries.ThresholdMovingTargetsMainRangeMargin = parm;
};
void		Config::setThresholdMovingTargetsLongRangeMargin(string  parm) {
	entries.ThresholdMovingTargetsLongRangeMargin = parm;
};

void		Config::setFirmwareVersion(string  parm) {
	entries.FirmwareVersion = parm;
};
void		Config::setSerialNumber(string  parm) {
	entries.SerialNumber = parm;
};
void		Config::setDspHardwareVersion(string  parm) {
	entries.DspHardwareVersion = parm;
};
void		Config::setRfeHardwareVersion(string  parm) {
	entries.RfeHardwareVersion = parm;
};
void		Config::setProductInfo(string  parm) {
	entries.ProductInfo = parm;;
};
void		Config::setTargetClusteringEnable(bool parm) {
	entries.TargetClusteringEnable = parm;
}
void		Config::setProcessingRcsCalibrationEnable(bool parm) {
	entries.ProcessingRcsCalibrationEnable = parm;
}

bool Config::isConfFileCorrupt() {
	return confile_corrupt;
}
bool Config::isDebugEnabled() {
	return debugenabled;
}
bool Config::populate(char  *filename) {
	if (!doesFileExist(filename)) {
		populated = false;
		printf("file %s didn't exist", filename);
		confile_corrupt = false;
		return true;
	}
	std::ifstream i(filename);
	try {
		json j;

		if (EOF == i.peek()) {
			cout << "config file empty ---- exiting -------" << endl;
			i.close();
			if (remove(filename) != 0) {
				cout << "Error deleting " << filename << endl;
			}
			confile_corrupt = true;
			return false; // nothing in the configuration
		}
		confile_corrupt = false;
		try {
			i >> j;
			fromJSON(j);
		}
		catch (json::parse_error exc) {
			cout << "JSON exception  " << exc.what() << endl;
		}
	}
	catch (exception err) {
		cout << "JSON exception " << err.what() << endl;
		return false;
	}
	return true;
}

Config * matchConfig(string serialnumber) {
	Config *cfg = nullptr;
	for (int i = 0; i < config_entries; i++) {
		cfg = cfgs[i];
		string sn = cfg->getSerialNumber();
		if (0 == sn.compare(serialnumber)) {
			return cfg;
		}
	}
	return nullptr;
}

bool 	Config::fromJSON(json &j) {
	//get the server
	for (auto& element : j) {
		if (element.is_array()) {
			//cout << "this element is an array ------------------------- " << endl;
			for (json::iterator it = element.begin(); it != element.end(); ++it) {
				try {
					// get the devices
					json j_device = *it;
					string serial_number = j_device[JS_SERIALNUMBER].get<std::string>();
					Config *found = matchConfig(serial_number);
					if (nullptr == found) continue;

					found->setFrequencyChannel(j_device[JS_FREQUENCY].get<std::string>());
					found->setx(j_device[JS_X].get<std::string>());
					found->sety(j_device[JS_Y].get<std::string>());
					found->setAngle(j_device[JS_ANGLE].get<std::string>());
					found->setThresholdMovingTargetsNearRangeMargin(j_device[JS_NEAR_RANGE_MARGIN].get<std::string>());
					found->setThresholdMovingTargetsMainRangeMargin(j_device[JS_MAIN_RANGE_MARGIN].get<std::string>());
					found->setThresholdMovingTargetsLongRangeMargin(j_device[JS_LONG_RANGE_MARGIN].get<std::string>());
					bool checked = j_device[JS_RCSCALIBRATION];
					found->setProcessingRcsCalibrationEnable(checked);
					checked = j_device[JS_CLUSTERING];
					found->setTargetClusteringEnable(checked);

				}
				catch (exception e) {
					cout << " Error processing config file " << e.what() << endl;
				}
			}
		}
		else {
			//std::cout << element << '\n';
			this->setmqttBroker(element[JS_MQTT_BROKER].get<std::string>());
			this->setmqttPort(element[JS_MQTT_PORT].get<std::string>());
			this->setmqttClient(element[JS_MQTT_CLIENT].get<std::string>());
			this->setmqttPassword(element[JS_MQTT_PASSWORD].get<std::string>());
			this->setmqttTopic(element[JS_MQTT_TOPIC].get<std::string>());
			this->setThresholdMovingTargetsNearRangeMargin(element[JS_NEAR_RANGE_MARGIN].get<std::string>());
			this->setNotify(element[JS_NOTIFY].get<std::string>());
			this->occupancy->pause();
			this->occupancy->setRangeMax(element[JS_RANGE_MAX]);
			this->occupancy->setRangeMin(element[JS_RANGE_MIN]);
			this->occupancy->setRangeSegmentSize(element[JS_RANGE_SEGMENT_SIZE]);
			this->occupancy->setAngleMax(element[JS_ANGLEMAX]);
			this->occupancy->setAngleSegmentSize(element[JS_ANGLE_SEGMENT_SIZE]);
			this->occupancy->setCellExpired(element[JS_CELL_EXPIRED]);
			this->occupancy->setCellMax(element[JS_CELL_MAX]);
			this->occupancy->setMaxVelocity(element[JS_MAX_VELOCITY].get<std::string>());
			this->occupancy->update();
		}
	}
	return true;
}


bool 	Config::toJSON(json &j) {

	json j_array;
	json j_server;
	json j_obj;
	j_server[JS_MQTT_BROKER] = this->mqttBroker;
	j_server[JS_MQTT_PORT] = this->mqttPort;
	j_server[JS_MQTT_CLIENT] = this->mqttClient;
	j_server[JS_MQTT_PASSWORD] = this->mqttPassword;
	j_server[JS_MQTT_TOPIC] = this->mqttTopic;
	j_server[JS_NOTIFY] = this->notify;
	j_server[JS_NEAR_RANGE_MARGIN] = this->getThresholdMovingTargetsNearRangeMargin();
	j_server[JS_RANGE_MAX] = this->occupancy->getRangeMax();//
	j_server[JS_RANGE_MIN] = this->occupancy->getRangeMin();
	j_server[JS_RANGE_SEGMENT_SIZE] = this->occupancy->getRangeSegmentSize();
	j_server[JS_ANGLEMAX] = this->occupancy->getAngleMax();
	j_server[JS_ANGLE_SEGMENT_SIZE] = this->occupancy->getAngleSegmentSize();
	j_server[JS_CELL_EXPIRED] = this->occupancy->getCellExpired();
	j_server[JS_CELL_MAX] = this->occupancy->getCellMax();
	j_server[JS_MAX_VELOCITY] = this->occupancy->getMaxVelocity();

	for (int i = 0; i < config_entries; i++)
	{
		Config *cfg = cfgs[i];
		j_obj[JS_FREQUENCY] = cfg->entries.FrequencyChannel;
		j_obj[JS_COMPORT] = cfg->entries.comPort;
		j_obj[JS_X] = cfg->entries.x;
		j_obj[JS_Y] = cfg->entries.y;
		j_obj[JS_ANGLE] = cfg->entries.angle;
		j_obj[JS_NEAR_RANGE_MARGIN] = cfg->entries.ThresholdMovingTargetsNearRangeMargin;
		j_obj[JS_MAIN_RANGE_MARGIN] = cfg->entries.ThresholdMovingTargetsMainRangeMargin;
		j_obj[JS_LONG_RANGE_MARGIN] = cfg->entries.ThresholdMovingTargetsLongRangeMargin;
		j_obj[JS_LIST_ENTRY] = cfg->entries.ListEntry;
		j_obj[JS_DEVICE_NAME] = cfg->entries.DeviceName;
		j_obj[JS_DEVICE_ADDRESS] = cfg->entries.DeviceAddress;
		j_obj[JS_RCSCALIBRATION] = cfg->entries.ProcessingRcsCalibrationEnable;
		j_obj[JS_CLUSTERING] = cfg->entries.TargetClusteringEnable;

		/* get firmware version */
		//cfg->setFirmwareVersion(major << "." << minor);
		//j["FirmwareVersion"] = cfg->getFirmwareVersion();

		/* get product info */
		j_obj[JS_PRODUCTINFO] = cfg->entries.ProductInfo;

		/* get serialnumber */

		j_obj[JS_SERIALNUMBER] = cfg->entries.SerialNumber;
		/* get DSP hw version from sensor */

		j_obj[JS_DSP_HARDWAREVERSION] = cfg->entries.DspHardwareVersion;

		/* get RFE hw version from sensor */

		j_obj[JS_RFE_HARDWAREVERSION] = cfg->entries.RfeHardwareVersion;
		j_array.push_back(j_obj);
	}
	j.push_back(j_server);
	j.push_back(j_array);
	return true;
}

void Config::webFormPrint(string &webform) {


	webform.append("<h2>MQTT Broker Config</h2>");
	webform.append("<form action = \"/a/config\" method=\"get\">");

	webform.append("<table style = \"width: 300px;\" border=\"black\">"
		"<tbody>");


	webform.append("<td>MQTT Broker<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \" "
		"value=\"");
	webform.append(this->mqttBroker);
	webform.append("\" name=\""
		JS_MQTT_BROKER
		"\"/>");
	webform.append("<tr>");
	webform.append("<td>Port<td>"
		"<input type=\"text\" size=5 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttPort);
	webform.append("\" name=\""
		JS_MQTT_PORT
		"\"/>");
	webform.append("<tr>");
	webform.append("<td>Client<td>"
		"<input type=\"text\" size=5 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttClient);
	webform.append("\" name=\""
		JS_MQTT_CLIENT
		"\"/>");
	webform.append("<tr>");
	webform.append("<td>Password<td>"
		"<input type=\"password\" size=5 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttPassword);
	webform.append("\" name=\""
		JS_MQTT_PASSWORD
		"\"/>");
	webform.append("<tr>");
	webform.append("<td>Topic<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->mqttTopic);
	webform.append("\" name=\""
		JS_MQTT_TOPIC
		"\"/>");

	webform.append("</tbody></table>");

	webform.append("<h2>Occupany Filter Config</h2><tr>");
	webform.append("<table border=\"black\">"
		"<tbody>");
	webform.append("<td>Max Range<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(to_string(this->occupancy->getRangeMax()));
	webform.append("\" name=\"");
	webform.append(JS_RANGE_MAX
		"\"/>");

	webform.append("<tr>");
	webform.append("<td>Min Range<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(to_string(this->occupancy->getRangeMin()));
	webform.append("\" name=\"");
	webform.append(JS_RANGE_MIN
		"\"/>");

	webform.append("<tr>");
	webform.append("<td>Range Step<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(to_string(this->occupancy->getRangeSegmentSize()));
	webform.append("\" name=\""
		JS_RANGE_SEGMENT_SIZE
		"\"/>");

	webform.append("<tr>");
	webform.append("<td>Alpha Max<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(to_string(this->occupancy->getAngleMax()));
	webform.append("\" name=\""
		JS_ANGLEMAX
		"\"/>");

	webform.append("<tr>");
	webform.append("<td>Alpha Step<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(to_string(this->occupancy->getAngleSegmentSize()));
	webform.append("\" name=\""
		JS_ANGLE_SEGMENT_SIZE
		"\"/>");
	webform.append("</td>");


	webform.append("<tr>");
	webform.append("<td>Threshold count (20ms)<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(to_string(this->occupancy->getCellExpired()));
	webform.append("\" name=\""
		JS_CELL_EXPIRED
		"\"/>");
	webform.append("</td>");


	webform.append("<tr>");
	webform.append("<td>Max Count<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(to_string(this->occupancy->getCellMax()));
	webform.append("\" name=\""
		JS_CELL_MAX
		"\"/>");
	webform.append("</td>");


	webform.append("<tr>");
	webform.append("<td>Maximum Velocity<td>"
		"<input type=\"text\" size=20 style=\"background-color: #808080; color: #ffffff; \"value=\"");
	webform.append(this->occupancy->getMaxVelocity());
	webform.append("\" name=\""
		JS_MAX_VELOCITY
		"\"/>");
	webform.append("</td>");

	webform.append("<tr>"
		"<td>");
	webform.append("Notify * 50ms</td>"
		"<td>");
	webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
	webform.append(this->notify.c_str());
	webform.append("\" name=\"");
	webform.append(JS_NOTIFY);
	webform.append("\">");

	webform.append("</td>");
	webform.append("</tbody></table>");
	webform.append("<h2>Sensor Config</h2>");

	webform.append("<br>");
	for (int i = 0; i < config_entries; i++)
	{
		Config *cfg = cfgs[i];
		webform.append("<div style=\"float:left\" >");

		webform.append("<table border=\"black\">"
			"<tbody>");

		/////////////////////////
		webform.append("<td>"
			"DeviceName</td><td><span style=\"color: #0000ff; \"/>");
		webform.append(cfg->entries.DeviceName.c_str());
		webform.append("</td>");

		//////////////////////////
		webform.append("<tr>"
			"<td>");
		webform.append("DeviceAddress<td>");
		webform.append(cfg->entries.DeviceAddress.c_str());
		webform.append("</td>");

		//////////////////////////
		/* get product info */
		webform.append("<tr>"
			"<td>");
		webform.append("ProductInfo</td><td>");
		webform.append(cfg->entries.ProductInfo.c_str());
		webform.append("</td>");

		/* get serialnumber */
		webform.append("<tr>"
			"<td>");
		webform.append("SerialNumber</td><td>");
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("</td>");

		/* get DSP hw version from sensor */

		webform.append("<tr>"
			"<td>");
		webform.append("DspHardwareVersion </td>"
			"<td>");
		webform.append(cfg->entries.DspHardwareVersion.c_str());
		webform.append("</td>");

		/* get RFE hw version from sensor */

		webform.append("<tr>"
			"<td>");
		webform.append("RfeHardwareVersion </td><td>");
		webform.append(cfg->entries.RfeHardwareVersion.c_str());
		webform.append("</td>");


		/*
		CommPort
		*/

		webform.append("<tr>"
			"<td>");
		webform.append("Comm Port </td><td>");
		webform.append(cfg->entries.comPort.c_str());
		webform.append("</td>");


		/*
		JS_X
		*/

		webform.append("<tr>"
			"<td>");
		webform.append("x</td>"
			"<td>");
		webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
		webform.append(cfg->entries.x.c_str());
		webform.append("\" name=\"");
		webform.append(JS_X);
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("\">");

		webform.append("</td>");

		/*
		JS_Y
		*/

		webform.append("<tr>"
			"<td>");
		webform.append("y</td>"
			"<td>");
		webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
		webform.append(cfg->entries.y.c_str());
		webform.append("\" name=\"");
		webform.append(JS_Y);
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("\">");

		webform.append("</td>");

		/*
		JS_ANGLE
		*/

		webform.append("<tr>"
			"<td>");

		webform.append("Angle</td>"
			"<td>");
		webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
		webform.append(cfg->entries.angle.c_str());
		webform.append("\" name=\"");
		webform.append(JS_ANGLE);
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("\">");

		webform.append("</td>");

		/*
		JS_FREQUENCY
		*/

		webform.append("<tr>"
			"<td>");

		webform.append("Frequency Channel </td>"
			"<td>");
		webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
		webform.append(cfg->entries.FrequencyChannel.c_str());
		webform.append("\" name=\"");
		webform.append(JS_FREQUENCY);
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("\">");

		webform.append("</td>");

		/*
		JS_NEAR_RANGE_MARGIN
		*/

		webform.append("<tr>"

			"<td>");
		webform.append("TargetsShortRangeMargin </td>"
			"<td>");
		webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \" size=5 value=\"");
		webform.append(cfg->entries.ThresholdMovingTargetsNearRangeMargin.c_str());
		webform.append("\" name=\"");
		webform.append(JS_NEAR_RANGE_MARGIN);
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("\">");

		webform.append("</td>");

		/*
		JS_MAIN_RANGE_MARGIN
		*/

		webform.append("<tr>"

			"<td>");
		webform.append("TargetsMainRangeMargin </td>"
			"<td>");
		webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \" size=5 value=\"");
		webform.append(cfg->entries.ThresholdMovingTargetsMainRangeMargin.c_str());
		webform.append("\" name=\"");
		webform.append(JS_MAIN_RANGE_MARGIN);
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("\">");

		webform.append("</td>");

		/*
		JS_LONG_RANGE_MARGIN
		*/

		webform.append("<tr>"

			"<td>");
		webform.append("TargetsLongRangeMargin </td>"
			"<td>");
		webform.append("<input type=\"text\" style=\"background-color: #808080; color: #ffffff; \"size=5 value=\"");
		webform.append(cfg->entries.ThresholdMovingTargetsLongRangeMargin.c_str());
		webform.append("\" name=\"");
		webform.append(JS_LONG_RANGE_MARGIN);
		webform.append(cfg->entries.SerialNumber.c_str());
		webform.append("\">");
		webform.append("</td>");

		/*
		JS_RCSCALIBRATION
		*/

		webform.append("<tr>");
		webform.append("<td>"
			"<input type = \"checkbox\" name = \"");
		string name_and_id = JS_RCSCALIBRATION;
		name_and_id.append(cfg->entries.SerialNumber);
		webform.append(name_and_id);
		webform.append("\" ");
		webform.append(" id=\"");
		webform.append(name_and_id);
		webform.append("\""
			" onclick=\"checkFunction(name)\" value = \"checkcheck box\")");
		if (cfg->entries.ProcessingRcsCalibrationEnable) {
			webform.append(" checked");
		}
		else {
			webform.append("\"");
		}
		webform.append(">RcsCalibrationEnable</td>");

		/*
		JS_CLUSTERING
		*/

		webform.append("<td>"
			"<input type = \"checkbox\" name = \"");
		name_and_id = JS_CLUSTERING;
		name_and_id.append(cfg->entries.SerialNumber);
		webform.append(name_and_id);
		webform.append("\" ");
		webform.append(" id=\"");
		webform.append(name_and_id);
		webform.append("\""
			" onclick=\"checkFunction(name)\" value = \"checkcheck box\"");
		if (cfg->entries.TargetClusteringEnable) {
			webform.append(" checked");
		}
		webform.append(">ClusteringEnable</td>");

		name_and_id = FORM_START_ACQUISITION;
		name_and_id.append(cfg->entries.SerialNumber);
		webform.append("<tr><td>"
			"<input type = \"submit\" value = \"Start Acquistion\" name = \"");
		webform.append(name_and_id);
		webform.append("\" />");
		webform.append("</td>");

		name_and_id = FORM_STOP_ACQUISITION;
		name_and_id.append(cfg->entries.SerialNumber);
		webform.append("<td>"
			"<input type = \"submit\" value = \"Stop Acquistion\" name = \"");
		webform.append(name_and_id);
		webform.append("\" />");
		webform.append("</td>");

		webform.append("</div>");

	}

	webform.append("</span>");
	webform.append("</tbody>"
		"</table>");

	webform.append(
		"<input type = \"submit\" value = \"Save Config\" name = \"config\" />"
		"<br><br>"
		"</form>");


	webform.append("<script>"
		"function checkFunction() {"
		"var arg0 = arguments[0];"
		"var elem = document.getElementById(arg0);"
		"elem.value = elem.checked == true ? \"checked\":\"notchecked\";"
		"var message = arg0 + \" checked: \" + elem.checked + \"  val:\" + elem.value;"
		"}"
		"</script>");

}

bool Config::webFormProcess(struct mg_connection *conn, string &s) {

	if (CivetServer::getParam(conn, JS_COMPORT, s)) {
		this->setcomPort(s);
	}
	if (CivetServer::getParam(conn, JS_MQTT_BROKER, s)) {
		this->setmqttBroker(s);
	}
	if (CivetServer::getParam(conn, JS_MQTT_PORT, s)) {
		this->setmqttPort(s);
	}
	if (CivetServer::getParam(conn, JS_MQTT_CLIENT, s)) {
		this->setmqttClient(s);
	}
	if (CivetServer::getParam(conn, JS_MQTT_PASSWORD, s)) {
		this->setmqttPassword(s);
	}
	if (CivetServer::getParam(conn, JS_MQTT_TOPIC, s)) {
		this->setmqttTopic(s);
	}
	if (CivetServer::getParam(conn, JS_RANGE_MAX, s)) {
		this->occupancy->setRangeMax(stoi(s));
	}
	if (CivetServer::getParam(conn, JS_RANGE_MIN, s)) {
		this->occupancy->setRangeMin(stoi(s));
	}
	if (CivetServer::getParam(conn, JS_RANGE_SEGMENT_SIZE, s)) {
		this->occupancy->setRangeSegmentSize(stoi(s));
	}
	if (CivetServer::getParam(conn, JS_ANGLEMAX, s)) {
		this->occupancy->setAngleMax(stoi(s));
	}
	if (CivetServer::getParam(conn, JS_ANGLE_SEGMENT_SIZE, s)) {
		this->occupancy->setAngleSegmentSize(stoi(s));
	}
	if (CivetServer::getParam(conn, JS_CELL_EXPIRED, s)) {
		this->occupancy->setCellExpired(stoi(s));
	}	
	if (CivetServer::getParam(conn, JS_CELL_MAX, s)) {
		this->occupancy->setCellMax(stoi(s));
	}
	if (CivetServer::getParam(conn, JS_MAX_VELOCITY, s)) {
		this->occupancy->setMaxVelocity(s);
	}
	if (CivetServer::getParam(conn, JS_NEAR_RANGE_MARGIN, s)) {
		this->setThresholdMovingTargetsNearRangeMargin(s);
	}
	if (CivetServer::getParam(conn, JS_MAIN_RANGE_MARGIN, s)) {
		this->setThresholdMovingTargetsNearRangeMargin(s);
	}
	if (CivetServer::getParam(conn, JS_LONG_RANGE_MARGIN, s)) {
		this->setThresholdMovingTargetsNearRangeMargin(s);
	}


	if (CivetServer::getParam(conn, JS_NOTIFY, s)) {
		this->setNotify(s);
	}
	for (int i = 0; i < config_entries; i++)
	{
		Config *cfg = cfgs[i];
		string param;
		param.append(JS_X);
		param.append(cfg->entries.SerialNumber);
		if (CivetServer::getParam(conn, param.c_str(), s)) {
			cfg->setx(s);
		}
		param.clear();
		param.append(JS_Y);
		param.append(cfg->entries.SerialNumber);

		if (CivetServer::getParam(conn, param.c_str(), s)) {
			cfg->sety(s);
		}
		param.clear();
		param.append(JS_ANGLE);
		param.append(cfg->entries.SerialNumber);

		if (CivetServer::getParam(conn, param.c_str(), s)) {
			cfg->setAngle(s);
		}
		param.clear();
		param.append(JS_FREQUENCY);
		param.append(cfg->entries.SerialNumber);

		if (CivetServer::getParam(conn, param.c_str(), s)) {
			cfg->setFrequencyChannel(s);
		}

		param.clear();
		param.append(JS_MAIN_RANGE_MARGIN);
		param.append(cfg->entries.SerialNumber);

		if (CivetServer::getParam(conn, param.c_str(), s)) {
			cfg->setThresholdMovingTargetsMainRangeMargin(s);
		}
		param.clear();
		param.append(JS_NEAR_RANGE_MARGIN);
		param.append(cfg->entries.SerialNumber);

		if (CivetServer::getParam(conn, param.c_str(), s)) {
			cfg->setThresholdMovingTargetsNearRangeMargin(s);
		}
		param.clear();
		param.append(JS_LONG_RANGE_MARGIN);
		param.append(cfg->entries.SerialNumber);

		if (CivetServer::getParam(conn, param.c_str(), s)) {
			cfg->setThresholdMovingTargetsLongRangeMargin(s);
		}

		param.clear();
		param.append(JS_RCSCALIBRATION);
		param.append(cfg->entries.SerialNumber);
		bool checked = false;
		if (CivetServer::getParam(conn, param.c_str(), s)) {
			checked = true;
		}
		cfg->setProcessingRcsCalibrationEnable(checked);
		param.clear();
		param.append(JS_CLUSTERING);
		param.append(cfg->entries.SerialNumber);
		checked = false;
		if (CivetServer::getParam(conn, param.c_str(), s)) {
			checked = true;
		}
		cfg->setTargetClusteringEnable(checked);
	}


	return true;
}

void startAcquisition(Config *cfg)
{
	iSYSResult res;
	/* start measurement */
	//cout << "start measurement.. \n";
	res = iSYS_StartAcquisition(cfg->getHandle(), 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not start measurement!\n";
	}
	return;
}
void stopAcquisition(Config *cfg)
{
	iSYSResult res;
	//cout << "stop measurement.. \n";
	res = iSYS_StopAcquisition(cfg->getHandle(), 100, 1000);
	if (res != ERR_OK) {
		cout << "error: " << ERROR_CODES[res] << "\n";
		cout << "can not stop measurement! \n";
	}

	/* NOTE start and stop measurement may take up to 500ms! */
	//cout << "wait for 500ms.. \n";
	Sleep(500);

	return;
}
bool Config::webAcquisitionProcess(struct mg_connection *conn, string &s) {

	for (int i = 0; i < config_entries; i++)
	{
		Config *cfg = cfgs[i];
		string param;

		param.clear();
		param.append(FORM_START_ACQUISITION);
		param.append(cfg->entries.SerialNumber);
		if (CivetServer::getParam(conn, param.c_str(), s)) {
			startAcquisition(cfg);
			mg_printf(conn, "got start for %s", cfg->getSerialNumber().c_str());
		}
		param.clear();
		param.append(FORM_STOP_ACQUISITION);
		param.append(cfg->entries.SerialNumber);
		if (CivetServer::getParam(conn, param.c_str(), s)) {
			stopAcquisition(cfg);
			mg_printf(conn, "got stop for %s", cfg->getSerialNumber().c_str());
		}

	}
	return true;
}



