/*
 * datatype.h
 *
 *  Created on: 22/03/2013
 *      Author: Dani
 */

#ifndef DATATYPE_H_
#define DATATYPE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <Windows.h>
#include <setupapi.h>
#include "lusb0_usb.h"

#define MAX_BUFFER			128


/* For libusb_set_configuration */
#define HID_CONFIGURATION               0x01

#define ENDPOINT_INT_IN                 0x81
#define ENDPOINT_INT_OUT                0x01
#define TIMEOUT                         100
#define MAX_DATA_REGIONS                 6
#define MAX_LONG_HEX	256


struct mowaysenddata
{
	uint8_t address;
	uint8_t command;
	uint8_t speed;
	uint8_t rotation;
	uint8_t time;
	uint8_t distance;
	uint8_t rotation_axis;
	uint8_t radius;
	uint8_t frequency;
};

struct rfusb{

	usb_dev_handle *devh;
	uint16_t vid; // VendorID
	uint16_t pid; // ProductID
	uint8_t state;
	uint8_t timeout;
	struct mowaysenddata mowaydata[MAX_BUFFER];
	struct mowaysenddata actual_data;
	int datain;
	int dataout;
};

struct moway_sensor{

	int obstacle[4];
	int line[2];
	float accel[3];
	int mic;
	int light;
	int mot_end;
	int distance;

};

//********************************************************************
//	Definitions
//********************************************************************
#pragma pack(1)
struct MEMORY_REGION
{
	unsigned char Type;
	DWORD Address;
	DWORD Size;
};

typedef union _BOOTLOADER_COMMAND
{
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char Pad[63];
	}EnterBootloader;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char Pad[63];
	}QueryDevice;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char BytesPerPacket;
		unsigned char DeviceFamily;
		struct MEMORY_REGION MemoryRegions[MAX_DATA_REGIONS];
		unsigned char Pad[8];
	}QueryResults;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char Setting;
		unsigned char Pad[62];
	}UnlockConfig;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char Pad[63];
	}EraseDevice;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		DWORD Address;
		unsigned char BytesPerPacket;
		unsigned char Data[58];
	}ProgramDevice;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char Pad[63];
	}ProgramComplete;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		DWORD Address;
		unsigned char BytesPerPacket;
		unsigned char Pad[58];
	}GetData;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		DWORD Address;
		unsigned char BytesPerPacket;
		unsigned char Data[58];
	}GetDataResults;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char Pad[63];
	}ResetDevice;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Command;
		unsigned char blockSize;
		unsigned char Data[63];
	}GetEncryptedFFResults;
	struct
	{
		unsigned char WindowsReserved;
		unsigned char Data[64];
	}PacketData;
	unsigned char RawData[65];
} BOOTLOADER_COMMAND;
#pragma pack()

struct usbmoway
{
	HANDLE WriteHandleRFUSB;
	HANDLE ReadHandleRFUSB;
	uint16_t vid; // VendorID
	uint16_t pid; // ProductID
	uint8_t state;
	int 	mowaybatt;
	int		firmware;
	int familia;  // PIC18, PIC24, PIC32
	int bytes_por_dir; // bytes by address(word width)
	int bytes_por_pack; // bytes per programming package

	uint8_t *vData[MAX_DATA_REGIONS];
	struct MEMORY_REGION *reg_mem;
	int regiones; // number of regions detected
	char archivo_hex[MAX_LONG_HEX]; // Name of the hex file

};

#endif /* DATATYPE_H_ */
