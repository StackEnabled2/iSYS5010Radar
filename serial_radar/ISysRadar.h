#pragma once
#define HAVE_STRUCT_TIMESPEC
#include "serial_radarAPI_if.h"
#include "pthread.h"
#include "pthreaddefs.h"
#include "serial_radarAPI_if.h"
//#include "async_publish.h"
#include "config.h"
#include "ISysSPI.h"


//const char ERROR_CODES[38][40] = {
const char ERROR_CODES[][40] = {
	"ERR_OK",
	"ERR_FUNCTION_DEPRECATED",
	"ERR_DLL_NOT_FINISHED",
	"ERR_HANDLE_NOT_INITIALIZED",
	"ERR_COMPORT_DOESNT_EXIST",
	"ERR_COMPORT_CANT_INITIALIZE",
	"ERR_COMPORT_ACCESS_DENIED",
	"ERR_COMPORT_BAUDRATE_NOT_VALID",
	"ERR_COMPORT_CANT_OPEN",
	"ERR_COMPORT_CANT_SET_FLOW_CONTROL",
	"ERR_COMPORT_CANT_SET_PARITY",
	"ERR_COMPORT_CANT_SET_STOP_BITS",
	"ERR_COMPORT_CANT_SET_DATA_BITS",
	"ERR_COMPORT_CANT_SET_BAUDRATE",
	"ERR_COMPORT_ALREADY_INITIALIZED",
	"ERR_COMPORT_EQUALS_NULL",
	"ERR_COMPORT_NOT_OPEN",
	"ERR_COMPORT_NOT_READABLE",
	"ERR_COMPORT_NOT_WRITEABLE",
	"ERR_COMPORT_CANT_WRITE",
	"ERR_COMPORT_CANT_READ",
	"ERR_COMMAND_NOT_WRITTEN ",
	"ERR_COMMAND_NOT_READ",
	"ERR_COMMAND_NO_DATA_RECEIVED",
	"ERR_COMMAND_NO_VALID_FRAME_FOUND",
	"ERR_COMMAND_RX_FRAME_DAMAGED",
	"ERR_COMMAND_FAILURE",
	"ERR_UNDEFINED_READ",
	"ERR_COMPORT_LESS_DATA_READ",
	"ERR_COMPORT_SYSTEM_INIT_FAILED",
	"ERR_COMPORT_SYSTEM_ALREADY_INITIALIZED",
	"ERR_COMMAND_RX_FRAME_LENGTH",
	"ERR_COMMAND_MAX_DATA_OVERFLOW",
	"ERR_COMMAND_MAX_IQPAIRS_OVERFLOW",
	"ERR_COMMAND_NOT_ACCEPTED",
	"ERR_NULL_POINTER",
	"ERR_CALC_CORRECTION_PARAMS",
	"ERR_PARAMETER_OUT_OF_RANGE"
};

#define DISCOVERY_RETRY 3
enum {
	LIST_BEGIN = 0
	, LIST_START
	, LIST_PROCESSING
	, LIST_END
	, LIST_ERROR
};

static bool start_acquisition = false;
static bool stop_acquisition = false;

class ISysRadar
{
public:
	ISysRadar();
	~ISysRadar();
	static bool start_acquisition;
	static bool stop_acquisition;
	static int scanPorts(Config *cfgs[], int max);
	iSYSResult attachSPIInterface();
	iSYSResult init();
	void * run(void  *pbuf);
	iSYSResult start();
	iSYSResult stop();
	iSYSResult startAcquisition();
	iSYSResult stopAcquisition();
	iSYSResult exitSystem();
	void setConfig(Config *cfg);
	Config *getConfig();
	iSYSResult_t getRadarConfig(bool update);
	void setPhandle(iSYSHandle *pHandle);
	iSYSHandle *getPhandle();
	static void *context_run(void *context)
	{
		return ((ISysRadar *)context)->run(nullptr);
	}
	string sensorConfig;

private:
	PUBClient *pubclient;
	Buf_t *buft = nullptr;
	ISysSPI *spi = nullptr;
	Config *cfg = nullptr;
	iSYSHandle_t pHandle;
	iSYSResult_t res;
	int deviceAddress;
	int notify_interval = 0;
	char * mqtt_server = nullptr;
	char * mqtt_client = nullptr;
	json jsonconfig;

	bool running = true;
	bool restart = false;

	uint8_t enable;

	iSYSFrequencyChannel_t radarchannel;
	uint8_t addr;
	iSYSTargetList *targetList = nullptr;

	bool myfunction(int i, int j);

	struct myclass {
		bool operator() (int i, int j) { return (i < j); }
	};
	struct myclass myobject;
	pthread_mutex_t buildlock;

private:

	iSYSResult_t decodeTargetFrame(uint8_t *pData
		, uint16_t nrOfElements
		, uint16_t productcode
		, uint8_t bitrate
		, uint8_t nrOfTargets
		, iSYSTargetList_t *targetList);

	void printTheList(char *filename, iSYSTargetList *targetList, char *mode, char *description, string data);

	int doesFileExist(const char *filename);
	bool list_start(uint8_t msg_check[], uint8_t &entries);
	bool list_end(uint8_t msg_check[]);
	void sortRange(iSYSTarget_t *targets, int number);
	void notify(iSYSTargetList_t *targetList, char desc[], bool print);
	void processParms(Buf_t *buf);
	iSYSResult processMessage(vector<unsigned char> msgBuf);
	void intHandler(int dummy);
	bool spiCheck(vector<unsigned char> &);

	iSYSResult decodeMessage(unsigned char * buf, uint16_t size, iSYSTargetList_t *targetList);
	iSYSResult startSensorConfig();

	float unpackFloat(const void *buf, int *i);

	iSYSResult stopAndStart();

	void printHex(uint8_t *buf, uint16_t size);

	iSYSResult_t initRadar();

	iSYSResult closeRadar();

};

