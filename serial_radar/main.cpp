#define HAVE_STRUCT_TIMESPEC

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <conio.h>
using namespace std;

#include "pthread.h"
#include "Config.h"
#include "serial_radarAPI_if.h"
#include "pthreaddefs.h"
#include "json.hpp"
#include "ISysRadar.h"
#include "ConfigPageHandler.h"

using namespace nlohmann;
ISysRadar *configureIsysRadar(ISysRadar *radar);
#define CONFIG_FILE "config.conf"
Config *main_cfg = nullptr;

int startConfigServer(int argc, char *argv[]);
int startPubClient(Config *cfg);
int startOccupancyFilter(Config *cfg);

void processParms(int argc, char *args[]);
int mqtt_port = 0;
char *mqtt_server = "";
char *mqtt_client = "";
char *mqtt_password = "";
char *mqtt_topic = "";

//Buf_t bufparm
#define NUM_THREADS 4
Buf_t thr_data[NUM_THREADS];
pthread_t thr[NUM_THREADS];

int config_entries = 0;
Config* cfgs[16];

int radar_entries=0;
ISysRadar * radars[4];
char desc[128] = { 0 };
int descindex = 0;

int disc_sesitivity = 20;
int disc_time = 500;
pthread_t webgo_thread;

Buf_t bufserver;

pthread_t pubclient_thread;
PUBClient *pub_client = nullptr;

pthread_t occupancy_thread;
Occupancy *occupancy = nullptr;

bool debugmsg = false;

int startThreads() {

	// now start the threads 
	for (int i = 0; i < config_entries; i++) {
		int rc = 0;
		ISysRadar *radar = radars[i];
		if (nullptr == radar) continue;
		if ((rc = pthread_create(&thr[i], NULL, &ISysRadar::context_run, radar))) {
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			return -1;
		}
	}

	return config_entries;
}


int main(int argc, char *argv[])
{
	char * argsx[7] = { "",
		"--port"
		, "9090"
		, "--mqtt"
		,"--mqttclient"
	, "--mqtt_port"
	, "7" };


	processParms(argc, argv);
	debugmsg = false;
	pub_client = new PUBClient();
	pub_client->setDebugMsg(debugmsg);
	occupancy = new Occupancy(debugmsg);
	occupancy->setDebugMsg(debugmsg);
	occupancy->init();
	occupancy->setPUBClient(pub_client);
	// get the com ports
	config_entries = ISysRadar::scanPorts(cfgs, sizeof cfgs);

	for (int i = 0; i < config_entries; i++) {
		Config *cfg = cfgs[i];
		cfg->setmqttBroker(mqtt_server);
		cfg->setmqttPort(to_string(mqtt_port));
		cfg->setmqttClient(mqtt_client);
		cfg->setmqttPassword(mqtt_password);
		cfg->setmqttTopic(mqtt_topic);
		cfg->setPUBClient(pub_client);
		cfg->setOccupancy(occupancy);
		iSYSHandle * pHandle = cfg->getHandle();
		if (nullptr == pHandle)  continue;
		ISysRadar *radar = new ISysRadar();
		radars[i] = radar;
		radar->setConfig(cfg);
		cfg->setHandle(pHandle);
		radar->setPhandle(pHandle);
		if (radar->getRadarConfig(false) != ERR_OK) { continue;  }
		radars[i] = radar;
	}

	main_cfg = new Config();

	main_cfg->setmqttBroker(mqtt_server);
	main_cfg->setmqttClient(mqtt_client);
	main_cfg->setmqttPassword(mqtt_password);
	main_cfg->setmqttTopic(mqtt_topic);
	main_cfg->setmqttPort(to_string(mqtt_port));
	main_cfg->setOccupancy(occupancy);
	main_cfg->setPUBClient(pub_client);

	main_cfg->populate(CONFIG_FILE);


	if (main_cfg->isConfFileCorrupt()) return 0;
	if (!main_cfg->isPopulated()) {
		iSYSResult res = ERR_OK;
		main_cfg->setNotify("5");
		main_cfg->createDefault(CONFIG_FILE);
	}	
	pub_client->setTopic(main_cfg->getmqttTopic());


	for (int i = 0; i < config_entries; i++) {
		ISysRadar *rdr = radars[i];

		configureIsysRadar(rdr);

		// change the margin for sensitivy
		iSYS_setThresholdMovingTargetsMainRangeMargin(rdr->getConfig()->getHandle(), ISYS_LOCATION_RAM, disc_sesitivity, 100, 1000);

		// configure the filter
		Config *devicecfg = cfgs[i];
		devicecfg->setOCCHandle(occupancy->addDevice(devicecfg->getSerialNumber()));


	}

	ISysSPI::openSPIPorts();

	for (int i = 0; i < config_entries; i++) {

		ISysRadar *rdr = radars[i];
		iSYSResult res;

		res = rdr->attachSPIInterface();
		if (res != ERR_OK) {
			cout << "error: " << ERROR_CODES[res] << "\n";
			cout << "Can't attach SPI Interface\n";
		}
	}

	// set the main range margins back
	for (int i = 0; i < config_entries; i++) {
		ISysRadar *rdr = radars[i];
		iSYS_setThresholdMovingTargetsMainRangeMargin(rdr->getConfig()->getHandle()
			, ISYS_LOCATION_RAM
			, stoi(rdr->getConfig()->getThresholdMovingTargetsMainRangeMargin()), 100, 1000);

	}
	pthread_setconcurrency(8);

	startConfigServer(argc, argv);


	startPubClient(main_cfg);

	startOccupancyFilter(main_cfg);

	

	int threads_started = startThreads();
	if (threads_started < 1) {
		cout << "Couldn't get all the radar threads started -- exit" << endl;
		return 0;
	}

	while (true) {
		// cycle the config server
		pthread_mutex_unlock(&bufserver.webgo);

		// check the keyboard
		if (_kbhit())
		{
			char in = 0x00;
			in = _getch();
			switch (in)
			{
				// carriage return
			case '\r':
				printf("\n");
				/*if (nullptr != targetList && targetList->nrOfTargets > 0) {
					printf("save the target list\n");
					desc[descindex] = '\0';

					notify(targetList, desc, true);

					//snap_a_picture(camera, filestring);

					memset(desc, 0, sizeof desc);
					descindex = 0;
				}
				if (nullptr != targetList) delete(targetList);
				targetList = nullptr;
				*/
				break;

				// backspace
			case 0x08:
			{
				printf("\b \b");
				desc[descindex != 0 ? descindex-- : descindex] = '\0';
				break;
			}

			// exit
			case '-':
			{
				//if (nullptr != targetList) delete(targetList);
				// targetList = nullptr;
				return ERR_OK;
			}
			// turn on/off messages
			case '+':
			{
				debugmsg = debugmsg == true ? false : true;
				occupancy->setDebugMsg(debugmsg);
				pub_client->setDebugMsg(debugmsg);
				break;
			}			
			// put it in the description string
			default:
				if (in != '\0') {
					desc[descindex++] = in;
					cout << in;
				}
			}
		}

		Sleep(50);
	}

}

int startIsysRadar() {
	return 0;
}


int startConfigServer(int argc, char *argv[])
{


	bufserver.argc = argc;
	bufserver.args = argv;
	for (int i = 0; i < argc; i++) {
		cout << argv[i] << endl;
	}
	pthread_mutex_init(&bufserver.webgo, NULL);

	if (pthread_create(&webgo_thread, NULL, config_server, &bufserver)) {

		fprintf(stderr, "Error creating radar thread\n");
		return 0;

	}

	return 1;
}

int startPubClient(Config *cfg) {

	pub_client->setBroker(cfg->getmqttBroker());
	pub_client->setClient(cfg->getmqttClient());
	pub_client->setPort(cfg->getmqttPort());
	pub_client->setTopic(cfg->getmqttTopic());
	pub_client->setMaxQSize(60);
	pub_client->setNotifyCycle(stoi(main_cfg->getNotify()));
	if (pthread_create(&pubclient_thread, NULL, &PUBClient::context_run, pub_client)) {

		fprintf(stderr, "Error creating pubclient thread\n");
		return 0;

	}
	return 1;
}

int startOccupancyFilter(Config *cfg) {



	if (pthread_create(&occupancy_thread, NULL, &Occupancy::context_run, occupancy)) {

		fprintf(stderr, "Error creating occupancy thread\n");
		return 0;

	}
	return 1;
}

void processParms(int argc, char *args[]) {

	for (int i = 0; i < argc; i++) {
		char **argslocal = args;
		cout << "parm serial radar " << args[i] << endl;
		if (0 == strcmp(args[i], "--mqtt_server")) {
			if (i + 1 < argc) {
				cout << " mqtt val:" << args[i + 1] << endl;
				mqtt_server = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add an mqtt server" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--mqtt_client")) {
			if (i + 1 < argc) {
				cout << " client val:" << args[i + 1] << endl;
				mqtt_client = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add an mqtt client" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--config")) {

			if (i + 1 < argc) {
				cout << " config val:" << args[i + 1] << endl;
				mqtt_client = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add a config file" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--mqtt_port")) {

			if (i + 1 < argc) {
				cout << " mqtt port val:" << args[i + 1] << endl;
				mqtt_port = atoi(args[i + 1]);
				i++;
			}
			else
			{
				cout << "need to add a config file" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--mqtt_topic")) {

			if (i + 1 < argc) {
				cout << " topic val:" << args[i + 1] << endl;
				mqtt_topic = args[i + 1];
				i++;
			}
			else
			{
				cout << "need to add a topic" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--disc_sensitivity")) {

			if (i + 1 < argc) {
				cout << " disc_sensitivity:" << args[i + 1] << endl;
				disc_sesitivity = atoi(args[i + 1]);
				i++;
			}
			else
			{
				cout << "need to add disc sensitivity" << endl;
			}
		}
		else if (0 == strcmp(args[i], "--disc_time")) {

			if (i + 1 < argc) {
				cout << " disc_time:" << args[i + 1] << endl;
				disc_time = atoi(args[i + 1]);
				i++;
			}
			else
			{
				cout << "need to add disc time" << endl;
			}
		}
	}
}

ISysRadar *configureIsysRadar(ISysRadar *radar) {
	iSYSResult res = ERR_OK;
	Config *cfg = radar->getConfig();
	//string comport = cfg->getcomPort();
	string serial = cfg->getSerialNumber();
	//cout << "trying to attach spi interface to com : " << comport.c_str() << " SerialNumber : " << serial.c_str() << endl;



	// init the radar unit
	res = radar->init();

	if (ERR_OK != res) {
		printf("Could not open the radar unit %s\n", ERROR_CODES[res]);
		return nullptr;
	}
	else {
		//cout << "Started RADAR Unit at COM" << cfg->getcomPort() << "  serial number: " << cfg->getSerialNumber() << endl;
	} 
	radar->getRadarConfig(false);
	//cout << radar->sensorConfig.c_str();

	//rdr->start();

	return radar;
}