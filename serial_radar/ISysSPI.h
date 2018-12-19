#pragma once
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <conio.h>
#include <signal.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include "ftd2xx.h"
#include "LibFT4222.h"



class ISysSPI
{
public:
	ISysSPI();
	virtual ~ISysSPI();

	static void ListFtUsbDevices();
	static void openSPIPorts();
	static ISysSPI *scanAlive();
	boolean ISysSPI::spiCheck(std::vector<unsigned char> &msgBuf, FT_HANDLE ftHandle);

	// FT_HANDLE init(int num);
	void start(Buf_t *buft);
	void stop();
	FT_HANDLE ftHandle = NULL;
private:
	std::vector< FT_DEVICE_LIST_INFO_NODE > g_FTAllDevList;
	std::vector< FT_DEVICE_LIST_INFO_NODE > g_FT4222DevList;

	uint16 rxSize;
	uint16 sizeTransferred;


	FT_STATUS ftStatus;
	FT_STATUS ft4222_status;
	bool keepRunning = true;
	void thread_loop(bool ready);

	FT_HANDLE spiInit(int pnum);
	//FT_HANDLE spiInit(ULONG pnum);
	static void flushBuffer(FT_HANDLE FTHandle);
	void spiClose();
};

//------------------------------------------------------------------------------
inline std::string DeviceFlagToString(DWORD flags)
{
	std::string msg;
	msg += (flags & 0x1) ? "DEVICE_OPEN" : "DEVICE_CLOSED";
	msg += ", ";
	msg += (flags & 0x2) ? "High-speed USB" : "Full-speed USB";
	return msg;
}
class SPIMessage
{
public:
	SPIMessage(uint16 size)
	{
		data.resize(size);

		for (uint16 i = 0; i < data.size(); i++)
		{
			data[i] = (uint8)i;
		}
	}

public:
	std::vector< unsigned char > data;
};
