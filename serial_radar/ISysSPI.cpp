#include <stdlib.h>
/*
 * @file spi_slave_test_slave_side.cpp
 *
 * @author FTDI
 * @date 2018-03-27
 *
 * Copyright 2011 Future Technology Devices International Limited
 * Company Confidential
 *
 * Revision History:
 * 1.0 - initial version
 * 1.1 - spi slave with protocol and ack function
  */
#define _REEENTRANT

  //------------------------------------------------------------------------------
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
#include <iostream>
//#include <fcntl.h>

#include "pthreaddefs.h"
#include "ISysSPI.h"
//------------------------------------------------------------------------------
// include FTDI libraries
//
#include "ftd2xx.h"
#include "LibFT4222.h"
#include "ISysRadar.h"

#define USER_WRITE_REQ      0x4a
#define USER_READ_REQ       0x4b

DWORD numOfDevices = 0;
DWORD numSPIObjects = 0;

ISysSPI *SPiEntries[4] = { nullptr };
int nextHandle = 0;

ISysSPI::ISysSPI()
{
}

ISysSPI::~ISysSPI()
{
}

void ISysSPI::flushBuffer(FT_HANDLE pftHandle)
{
	FT_STATUS ftStatus;
	uint16 rxSize;
	while (1) {
		ftStatus = FT4222_SPISlave_GetRxStatus(pftHandle, &rxSize);
		if (ftStatus == FT4222_OK && rxSize > 0)
		{
			uint16_t sizeTransferred;
			std::vector<unsigned char> tmpBuf;
			tmpBuf.resize(rxSize);

			ftStatus = FT4222_SPISlave_Read(pftHandle, &tmpBuf[0], rxSize, &sizeTransferred);
			if ((ftStatus == FT4222_OK) && (rxSize == sizeTransferred))
			{
				cout << pftHandle << "  -- flushed --  ";
				//FT4222_SPI_Reset(pftHandle);
				break;
			}
			else
			{
				printf("FT4222_SPISlave_Read error ft4222_status=%d\n", ftStatus);
				fprintf(stderr, "ERR from SPI %s\n", ERROR_CODES[ftStatus]);
				break;
			}
		}
		break;
	}

}

void ISysSPI::openSPIPorts() {

	bool found = false;
	FT_STATUS ftStatus = 0;
	//FT_HANDLE pftHandle = nullptr;

	ftStatus = FT_CreateDeviceInfoList(&numOfDevices);
	uint16 rxSize = 0;

	//ListFtUsbDevices();
	for (DWORD iDev = 0; iDev < numOfDevices; ++iDev)
	{
		FT_DEVICE_LIST_INFO_NODE devInfo;
		memset(&devInfo, 0, sizeof(devInfo));

		ftStatus = FT_GetDeviceInfoDetail(iDev, &devInfo.Flags, &devInfo.Type, &devInfo.ID, &devInfo.LocId,
			devInfo.SerialNumber,
			devInfo.Description,
			&devInfo.ftHandle);
		if (0 == strcmp(devInfo.SerialNumber, "A"))
		{
			ISysSPI *spi = new ISysSPI();
//			spi->ftHandle = spi->spiInit(iDev);
			spi->spiInit(iDev);
			cout << "received SPI handle from A " << spi->ftHandle << " loc: " << iDev <<  endl;

			SPiEntries[numSPIObjects++ ] = spi;
			
		}
	}
	//ListFtUsbDevices();
}

FT_STATUS resetTranscaction(FT_HANDLE hdl) {
		return FT4222_SPI_ResetTransaction(hdl, 0);
}

ISysSPI *ISysSPI::scanAlive() {
	bool found = false;
	FT_STATUS ftStatus = 0;
	uint16 rxSize = 0;
	ISysSPI *spi = nullptr;
	for (DWORD j = 0; j < numSPIObjects; j++) {
		spi = SPiEntries[j];
		if (nullptr == spi) continue;

		cout << "waiting " << j << " with fthandle " << spi->ftHandle<< endl;
		//flushBuffer(spi->ftHandle);
		ftStatus = resetTranscaction(spi->ftHandle);
		//ftStatus = FT4222_SPI_Reset(spi->ftHandle);
		if (ftStatus != FT4222_OK) {
			cout << "Couldn't reset transaction " << ftStatus << endl;
		}
		Sleep(disc_time);
		rxSize = 0;

		ftStatus = FT4222_SPISlave_GetRxStatus(spi->ftHandle, &rxSize);

		if (ftStatus == FT4222_OK && rxSize > 0)
		{
			found = false;
			uint16_t sizeTransferred;
			std::vector<unsigned char> tmpBuf;
			tmpBuf.resize(rxSize);

			ftStatus = FT4222_SPISlave_Read(spi->ftHandle, &tmpBuf[0], rxSize, &sizeTransferred);
			if ((ftStatus == FT4222_OK) && (rxSize == sizeTransferred))
			{
				found = false;
			}
			else
			{
				printf("FT4222_SPISlave_Read error ft4222_status=%d\n", ftStatus);
				fprintf(stderr, "ERR from SPI %s\n", ERROR_CODES[ftStatus]);
				break;
			}
			cout << "Received " << rxSize
				<< " bytes"
				<< "  at " << j << " handle " << spi->ftHandle << " --> nope" << endl;
			found = false;
		}
		else {
			found = true;
			//SPiEntries[j] = nullptr;
			break;
		}
	}
	if (!found) spi = nullptr;
	return spi;
}


void ISysSPI::ListFtUsbDevices()
{
	FT_STATUS ftStatus = 0;

	DWORD numOfDevices = 0;
	ftStatus = FT_CreateDeviceInfoList(&numOfDevices);

	for (DWORD iDev = 0; iDev < numOfDevices; ++iDev)
	{
		FT_DEVICE_LIST_INFO_NODE devInfo;
		memset(&devInfo, 0, sizeof(devInfo));

		ftStatus = FT_GetDeviceInfoDetail(iDev, &devInfo.Flags, &devInfo.Type, &devInfo.ID, &devInfo.LocId,
			devInfo.SerialNumber,
			devInfo.Description,
			&devInfo.ftHandle);

		if (FT_OK == ftStatus)
		{
			printf("Dev %d:\n", iDev);
			printf("  Flags= 0x%x, (%s)\n", devInfo.Flags, DeviceFlagToString(devInfo.Flags).c_str());
			printf("  Type= 0x%x\n", devInfo.Type);
			printf("  ID= 0x%x\n", devInfo.ID);
			printf("  LocId= 0x%x\n", devInfo.LocId);
			printf("  SerialNumber= %s\n", devInfo.SerialNumber);
			printf("  Description= %s\n", devInfo.Description);
			printf("  ftHandle= 0x%X\n", devInfo.ftHandle);

			const std::string desc = devInfo.Description;
			/*			g_FTAllDevList.push_back(devInfo);

						if (desc == "FT4222" || desc == "FT4222 A")
						{
							g_FT4222DevList.push_back(devInfo);
						}
			*/
		}
	}
}

void ISysSPI::start(Buf_t *buft) {
	Sleep(10);
}

void ISysSPI::stop() {
}


FT_HANDLE ISysSPI::spiInit(int pnum)
//FT_HANDLE ISysSPI::spiInit(ULONG locid)
{
	int num = pnum;
	num = pnum;
	ftStatus = FT_Open(num, &this->ftHandle);
	if (FT_OK != ftStatus)
	{
		printf("Open a FT4222 device failed status:%lu  num:%d\n", ftStatus, num);
		return nullptr;
	}

	ftStatus = FT4222_SetClock(this->ftHandle, SYS_CLK_80);
	if (FT_OK != ftStatus)
	{
		printf("FT4222_SetClock failed! %lu\n", ftStatus);
		return nullptr;
	}

	//Set default Read and Write timeout 1 sec
	ftStatus = FT_SetTimeouts(this->ftHandle, 1000, 1000);

	if (FT_OK != ftStatus)
	{
		printf("FT_SetTimeouts failed!  %lu\n", ftStatus);
		return nullptr;
	}

	// set latency to 1
	ftStatus = FT_SetLatencyTimer(this->ftHandle, 1);
	if (FT_OK != ftStatus)
	{
		printf("FT_SetLatencyTimerfailed!  %lu\n", ftStatus);
		return nullptr;
	}

	//
	ftStatus = FT_SetUSBParameters(this->ftHandle, 64 * 1024, 0);
	if (FT_OK != ftStatus)
	{
		printf("FT_SetUSBParameters failed!  %lu\n", ftStatus);
		return nullptr;
	}


	ft4222_status = FT4222_SPISlave_InitEx(this->ftHandle, SPI_SLAVE_NO_PROTOCOL);
	if (FT4222_OK != ft4222_status)
	{
		printf("Init FT4222 as SPI slave device failed!  %lu\n", ftStatus);
		return nullptr;
	}

	if (FT4222_OK != FT4222_SPISlave_SetMode(this->ftHandle, CLK_IDLE_LOW, CLK_TRAILING))  //// works ////
	{
		printf("Init FT4222 as SPI set mode failed!  %lu\n", ftStatus);
		return nullptr;
	}

	ft4222_status = FT4222_SPI_SetDrivingStrength(this->ftHandle, DS_4MA, DS_4MA, DS_4MA);
	if (FT4222_OK != ft4222_status)
	{
		printf("FT4222_SPI_SetDrivingStrength failed!  %lu\n", ftStatus);
		return nullptr;
	}

	/*	GPIO_Dir gpioDir[4];

		gpioDir[0] = GPIO_OUTPUT;
		gpioDir[1] = GPIO_OUTPUT;
		gpioDir[2] = GPIO_OUTPUT;
		gpioDir[3] = GPIO_OUTPUT;
		bool toggle = true;
		for (int idx = 0; idx < 20; idx++)
		{
			if (toggle)
				FT4222_GPIO_Write(this->ftHandle, GPIO_PORT3, 1);
			else
				FT4222_GPIO_Write(this->ftHandle, GPIO_PORT3, 0);

			toggle = !toggle;
			Sleep(500);
		}
		*/
	return this->ftHandle;
}

void ISysSPI::thread_loop(bool ready) {

	if (ready) {
		//pthread_mutex_unlock(&mutex_spi_ready);
	}
}

static uint64_t last_time = 0;
static uint16_t dots = 0;
boolean ISysSPI::spiCheck(std::vector<unsigned char> &msgBuf, FT_HANDLE ftH) {

	ft4222_status = FT4222_SPISlave_GetRxStatus(ftH, &rxSize);
	if (ft4222_status == FT4222_OK)
	{
		if (rxSize > 0)
		{
			std::chrono::high_resolution_clock m_clock;

			uint64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>
				(m_clock.now().time_since_epoch()).count();

			last_time = millis;
			std::vector<unsigned char> tmpBuf;
			msgBuf.resize(rxSize);

			ft4222_status = FT4222_SPISlave_Read(ftH, &msgBuf[0], rxSize, &sizeTransferred);
			if ((ft4222_status == FT4222_OK) && (rxSize == sizeTransferred))
			{
				return true;
			}
			else
			{
				printf("FT4222_SPISlave_Read error ft4222_status=%d\n", ft4222_status);
				fprintf(stderr, "ERR from SPI %s\n", ERROR_CODES[ft4222_status]);
				return false;
			}
		}
		else {
			return false;
		}
		return false;
	}
	return false;
}

void ISysSPI::spiClose()
{

	FT4222_UnInitialize(this->ftHandle);
	FT_Close(this->ftHandle);
}



