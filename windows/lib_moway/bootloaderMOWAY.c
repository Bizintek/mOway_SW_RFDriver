/*
 * bootloaderMOWAY.c
 *
 *  Created on: 26/03/2013
 *      Author: Dani
 */

#define _CRT_SECURE_NO_WARNINGS

#include "bootloaderMOWAY.h"
#include "datatype.h"
#include <setupapi.h>



extern struct usbmoway moway;
HINSTANCE hsetup, huser,hHID;

typedef struct _HIDD_ATTRIBUTES {
	    ULONG   Size;
	    USHORT  VendorID;
	    USHORT  ProductID;
	    USHORT  VersionNumber;
} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

typedef HDEVINFO  (WINAPI* SetupDiGetClassDevsUM)(CONST GUID*,PCTSTR,HWND,DWORD);
typedef WINSETUPAPI BOOL   (WINAPI* SetupDiEnumDeviceInterfacesUM)(HDEVINFO,PSP_DEVINFO_DATA,LPGUID,DWORD,PSP_DEVICE_INTERFACE_DATA);
typedef WINSETUPAPI BOOL   (WINAPI *SetupDiDestroyDeviceInfoListUM)(HDEVINFO);
typedef WINSETUPAPI BOOL   (WINAPI *SetupDiGetDeviceRegistryPropertyUM)(HDEVINFO,SP_DEVINFO_DATA,DWORD,PDWORD,PBYTE,DWORD,PDWORD);
typedef WINSETUPAPI BOOL   (WINAPI *SetupDiEnumDeviceInfoUM)(HDEVINFO,DWORD,PSP_DEVINFO_DATA);
typedef BOOL (WINAPI* SetupDiGetDeviceInterfaceDetailUM)(HDEVINFO,PSP_DEVICE_INTERFACE_DATA,PSP_DEVICE_INTERFACE_DETAIL_DATA,DWORD,PDWORD,PSP_DEVINFO_DATA);
typedef HDEVNOTIFY  (*RegisterDeviceNotificationUM)(HANDLE,LPVOID,DWORD);
typedef BOOLEAN (__stdcall* HidD_GetAttributesUM)(IN HANDLE HidDeviceObject,OUT PHIDD_ATTRIBUTES Attributes);

SetupDiGetClassDevsUM _SetupDiGetClassDevsUM;
SetupDiEnumDeviceInterfacesUM _SetupDiEnumDeviceInterfacesUM;
SetupDiDestroyDeviceInfoListUM _SetupDiDestroyDeviceInfoListUM;
SetupDiGetDeviceRegistryPropertyUM _SetupDiGetDeviceRegistryPropertyUM;
SetupDiEnumDeviceInfoUM _SetupDiEnumDeviceInfoUM;
SetupDiGetDeviceInterfaceDetailUM _SetupDiGetDeviceInterfaceDetailUM;
RegisterDeviceNotificationUM _RegisterDeviceNotificationUM;
HidD_GetAttributesUM _HidD_GetAttributesUM;

GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}; 

//********************************************************************
//	Program Moway
//********************************************************************
void ProgramMOWAY()
{
	BOOTLOADER_COMMAND myCommand;
	int ret;
	unsigned char* p;
	DWORD address;
	
	unsigned char i,currentByteInAddress,currentMemoryRegion;
	boolean configsProgrammed=FALSE,everythingElseProgrammed=FALSE,skipBlock,blockSkipped;

	everythingElseProgrammed = FALSE;
	configsProgrammed = TRUE;

	while((configsProgrammed == FALSE) || (everythingElseProgrammed == FALSE))
	{
		for(currentMemoryRegion=0;currentMemoryRegion<moway.regiones;currentMemoryRegion++)
		{
			if(configsProgrammed == FALSE)
			{
				if(moway.reg_mem[currentMemoryRegion].Type != MEMORY_REGION_CONFIG)
				{
					continue;
				}
			}
			else
			{
				if(moway.reg_mem[currentMemoryRegion].Type == MEMORY_REGION_CONFIG)
				{
					continue;
				}
			}
			address = moway.reg_mem[currentMemoryRegion].Address;
			
			p = getDataMemoryRegion(currentMemoryRegion);
			skipBlock = TRUE;
			blockSkipped = FALSE;
			currentByteInAddress = 1;

			while((address < (moway.reg_mem[currentMemoryRegion].Address + moway.reg_mem[currentMemoryRegion].Size)))
			{
				myCommand.ProgramDevice.WindowsReserved = 0;
				myCommand.ProgramDevice.Command = PROGRAM_DEVICE;
				myCommand.ProgramDevice.Address = address;

				for(i=0;i<moway.bytes_por_pack;i++)
				{
					unsigned char data;
					data = *p++;
					myCommand.ProgramDevice.Data[i+(sizeof(myCommand.ProgramDevice.Data)-moway.bytes_por_pack)] = data;
					#if !defined(ENCRYPTED_BOOTLOADER)
					if(data != 0xFF)
					{
						if(moway.bytes_por_dir == 2)
						{
							if((address%2)!=0)
							{
								if(currentByteInAddress == 2)
								{
								}
								else
								{
									skipBlock = FALSE;
								}
							}
							else
							{
								skipBlock = FALSE;
							}
						}
						else
						{
							skipBlock = FALSE;
						}
					}
					#else
					if(data != encryptedFF[i%encryptionBlockSize])
					{
						skipBlock = FALSE;
					}
					#endif
					if(currentByteInAddress == moway.bytes_por_dir)
					{
						address++;
						currentByteInAddress = 1;
					}
					else
					{
						currentByteInAddress++;
					}
					if((address >= (moway.reg_mem[currentMemoryRegion].Address + moway.reg_mem[currentMemoryRegion].Size)))
					{
						unsigned char n;
						i++;
						for(n=0;n<sizeof(myCommand.ProgramDevice.Data);n++)
						{
							if(n<i)
							{
								myCommand.ProgramDevice.Data[sizeof(myCommand.ProgramDevice.Data)-n-1] = myCommand.ProgramDevice.Data[i+(sizeof(myCommand.ProgramDevice.Data)-moway.bytes_por_pack)-n-1];
							}
							else
							{
								myCommand.ProgramDevice.Data[sizeof(myCommand.ProgramDevice.Data)-n-1] = 0;
							}
						}
						break;
					}
				}
				myCommand.ProgramDevice.BytesPerPacket = i;

				if(skipBlock == FALSE)
				{
					if(blockSkipped == TRUE)
					{
						blockSkipped = FALSE;
					}
					ret=WriteDataMOWAY( myCommand);
					skipBlock = TRUE;
				}
				else
				{
					blockSkipped = TRUE;
					skipBlock = TRUE;
				}
			}
			myCommand.ProgramDevice.WindowsReserved = 0;
			myCommand.ProgramComplete.Command = PROGRAM_COMPLETE;
			ret=WriteDataMOWAY( myCommand);
		}
		if(configsProgrammed == FALSE)
		{
			configsProgrammed = TRUE;
		}
		else
		{
			everythingElseProgrammed = TRUE;
		}
	}
}
//********************************************************************
//	Open HEX File chanel
//********************************************************************
int OpenHexFileMOWAY(const char *strData, uint8_t channel)
{
	char fileSupportBuffer[MAX_LONG_LIN], *str, *dataPayload,Buffer[MAX_LONG_LIN], channel_str[3];
	FILE *fileSupportFileStream;
	unsigned char i,j,*p, *limit;
	uint8_t *pData;
	unsigned long extendedAddress, recordLength, addressField,recordType,checksum,totalAddress,data;
	unsigned checksumCalculated,line=0;
	boolean  hexFileError=FALSE;
	
	sprintf(channel_str,"%02x",channel);

	if((fileSupportFileStream = fopen(strData, "r"))  == NULL)
	{
		return -1;
	}
	while (NULL != fgets(Buffer, MAX_LONG_LIN, fileSupportFileStream) && hexFileError == FALSE)
	{
		Buffer[strlen(Buffer)-1]='\0';
		for(j=0; j<strlen(Buffer); j++)
		{
			fileSupportBuffer[j]=Buffer[j];
		}
		if(fileSupportBuffer[0] != ':')
		{
			hexFileError = TRUE;
		}
		else
		{
			str = &(fileSupportBuffer[1]);

		}
		recordLength = HexToInt(substr(str,0, 2));
		addressField = HexToInt(substr(str,2, 4));
		recordType = HexToInt(substr(str,6,2));
		dataPayload = substr(str,8,recordLength*2);
		checksum = HexToInt(substr(str,(recordLength*2)+8,2));
		checksumCalculated=0;
		for(j=0;j<(recordLength+4);j++)
		{
			checksumCalculated += HexToInt(substr(str,j*2,2));
		}
		checksumCalculated = (~checksumCalculated) + 1;
		if((checksumCalculated & 0x000000FF) != checksum)
		{
			hexFileError = TRUE;
		}
		if (line == CH_LINE && channel <=125)
		{
			dataPayload[CH_POSITION] = channel_str[0];
			dataPayload[CH_POSITION + 1] = channel_str[1];
		}
		if(hexFileError == FALSE)
		{
			switch(recordType)
			{
				case HEX_FILE_EXTENDED_LINEAR_ADDRESS:
					extendedAddress = HexToInt(dataPayload);
				break;

				break;
				case HEX_FILE_DATA:
				{
					totalAddress = (extendedAddress << 16) + addressField;
					for(i=0;i<moway.regiones;i++)
					{

						pData = getDataMemoryRegion(i);
						if((totalAddress >= (moway.reg_mem[i].Address * moway.bytes_por_dir)) && (totalAddress <
								((moway.reg_mem[i].Address + moway.reg_mem[i].Size) * moway.bytes_por_dir)))
						{
							for(j=0;j<(recordLength);j++)
							{
								p = (unsigned char*)((totalAddress-(moway.reg_mem[i].Address * moway.bytes_por_dir)) + j);
								data = HexToInt(substr(dataPayload,j*2,2));
								p = (unsigned char*)(pData + (totalAddress-(moway.reg_mem[i].Address * moway.bytes_por_dir)) + j);
								limit = (unsigned char*)(pData + ((moway.reg_mem[i].Size + 1)*moway.bytes_por_dir));
								if(p>=limit)
								{
									break;
								}
								if(moway.bytes_por_dir == 2)
								{
								#if !defined(ENCRYPTED_BOOTLOADER)
								if((totalAddress + j) < 0x6)
								{
									unsigned char k, *userResetVectorPtr;
									switch((totalAddress+j))
									{
										case 0:
										*pData = 0x00;
										break;
										case 1:
										case 2:
										*(pData + j) = 0x04;
										break;
										case 3:
										case 4:
										case 5:
										*(pData + j) = 0x00;
										default:
										*pData = (unsigned char)(data);
										break;
									}
									for(k=0;k<moway.regiones;k++)
									{
										if(moway.reg_mem[k].Address == PIC24_RESET_REMAP_OFFSET)
										{
											pData = getDataMemoryRegion(k);
											userResetVectorPtr = (unsigned char*)(pData + totalAddress+j);
											*userResetVectorPtr = (unsigned char)(data);
											pData = getDataMemoryRegion(i);
										}
									}
								}
								else if((totalAddress+j)==(PIC24_RESET_REMAP_OFFSET*2))
								{
								}
								else if((totalAddress+j)==((PIC24_RESET_REMAP_OFFSET*2)+1))
								{
								}
								else if((totalAddress+j)==((PIC24_RESET_REMAP_OFFSET*2)+2))
								{
								}
								else if((totalAddress+j)==((PIC24_RESET_REMAP_OFFSET*2)+3))
								{
								}
								else if((totalAddress+j)==((PIC24_RESET_REMAP_OFFSET*2)+4))
								{
								}
								else if((totalAddress+j)==((PIC24_RESET_REMAP_OFFSET*2)+5))
								{
								}
								else
								#endif
								{
									*p = (unsigned char)(data);
								}
								}
								else
								{
									*p = (unsigned char)(data);
								}
							}
							break;
						}
					}
					break;
				}
				default:
				break;
			}
		}
		else
		{
			hexFileError = TRUE;
			printf("ERROR: Error in hex file somewhere\n");

		}
		line++;
	}
	fclose(fileSupportFileStream);
	
	if (hexFileError == TRUE)
		return -1;
	else
		return 0;
}
//********************************************************************
//	Read batt and firmware from MOWAY Robot
//********************************************************************
int ReadBattAndFirmwareMOWAY()
{
	BOOTLOADER_COMMAND Command = {0};
	BOOTLOADER_COMMAND Response = {0};
	DWORD ErrorStatus = ERROR_SUCCESS; 
	int mOwBat,mOwFirm;
	int adc_10bits;
	int batt_vol;

	//Command to "ask" the robot for its battery level and firmware version
	Command.ResetDevice.WindowsReserved = 0;
	Command.ResetDevice.Command = MOW_BATT;

	//Send the command to the robot
	if (WriteDataMOWAY( Command)==65)
	{
		//Read the data sent by the robot
		if (ReadDataMOWAY( &Response) == 65)
		{
			mOwBat=0;
			mOwFirm=0;
			adc_10bits= Response.GetData.Pad[0]+(Response.GetData.Pad[1]*256);
			batt_vol=(((adc_10bits*660)/1024));
			if(Response.GetData.Pad[1]==BATT_FULL)
				mOwBat=100;
			else if(batt_vol>=BATT_MIN){
				batt_vol=((batt_vol-BATT_MIN)*100)/(BATT_MAX-BATT_MIN);
				if(batt_vol>=100)
					mOwBat=99;
				else
					mOwBat=batt_vol;
			}
			else
				mOwBat=0;
			mOwFirm=Response.GetData.Pad[2];
		
			moway.mowaybatt=mOwBat;
			moway.firmware=mOwFirm;
			return 0;
		}
	}
	return -1;
}
//********************************************************************
//	Erase Flash MOWAY Robot
//********************************************************************
int EraseMOWAY()
{
	BOOTLOADER_COMMAND cmd,rta;
	int i;

	cmd.EraseDevice.WindowsReserved=0;
	cmd.EraseDevice.Command = ERASE_DEVICE;

	if(WriteDataMOWAY(cmd) != 65)
		printf("ERROR: Erase operation");

	for (i=0; i<30; i++) {
		delay_sec(1);
		cmd.QueryDevice.WindowsReserved=0;
		cmd.QueryDevice.Command = QUERY_DEVICE;
		if(WriteDataMOWAY(cmd) != 65)
			continue;
		if(ReadDataMOWAY(&rta)!=65)
			continue;
		
		return 0;
	}

	printf("ERROR: Erase operation");
	return -1;
}
//********************************************************************
//	Information MOWAY
//********************************************************************
void InformationMOWAY()
{
	int n = 0;
	BOOTLOADER_COMMAND Command,rta;
	int i,j,bytes;
	int length_mem;
	int c,b;

	for (i=0;i<64;i++) {
		Command.RawData[i]=0;
		rta.RawData[i]=0;
	}
	Command.ResetDevice.WindowsReserved = 0;
	Command.ResetDevice.Command = QUERY_DEVICE;
	bytes=WriteDataMOWAY(Command);
	if( bytes != 65)
	{
		printf("Error bulk_transfer");
		return;
	}
	bytes=ReadDataMOWAY(&rta);
	if( bytes != 65)
	{
		printf("Error bulk_transfer");
		return;
	}

	moway.regiones = 0;
	for (i=0; i<MAX_DATA_REGIONS; i++) {
		if (rta.QueryResults.MemoryRegions[i].Type == MEMORY_REGION_END)
		  { printf("Region:");
		    printf("%d",i+1);
		    printf("   tipo de region:");
		    c = (int)(rta.QueryResults.MemoryRegions[i].Type);
		    printf("%d",c);
		    printf("   tamaño de la region:");
		    printf("%d\n",rta.QueryResults.MemoryRegions[i].Size);
			 c = (int)(rta.QueryResults.MemoryRegions[i].Type);
		    printf("Pos de memoria de la region: ");
		    printf("%x\n",rta.QueryResults.MemoryRegions[i].Address);
			printf("\n\n");
		    break;}
		printf("Region:");
		printf("%d",i+1);
		printf("   tipo de region:");
		b = (int)(rta.QueryResults.MemoryRegions[i].Type);
		printf("%d",b);
		printf("   tamaño de la region:" );
		printf("%d\n",rta.QueryResults.MemoryRegions[i].Size);
		c = (int)(rta.QueryResults.MemoryRegions[i].Type);
		printf("Pos de memoria  de la region:");
		printf("%x\n ",rta.QueryResults.MemoryRegions[i].Address);
		printf("\n\n");

		moway.reg_mem[i].Type = rta.QueryResults.MemoryRegions[i].Type;
		moway.reg_mem[i].Address = rta.QueryResults.MemoryRegions[i].Address;
		moway.reg_mem[i].Size = rta.QueryResults.MemoryRegions[i].Size;
		moway.regiones++;
	}

	switch (rta.QueryResults.DeviceFamily) {
	case DEVICE_FAMILY_PIC18:
		moway.bytes_por_dir = 1;
		moway.familia = 18;
		break;
	case DEVICE_FAMILY_PIC24:
		moway.bytes_por_dir = 2;
		moway.familia = 24;
		break;
	case DEVICE_FAMILY_PIC32:
		moway.bytes_por_dir = 1;
		moway.familia = 32;
		break;
	}

	for (i=0; i<moway.regiones; i++)
	{
		if (moway.vData[i]!=NULL)
			free(moway.vData[i]);
	}

	for (i=0; i<moway.regiones; i++) {
		uint8_t *pData = (uint8_t*)
			malloc((moway.reg_mem[i].Size + 1) * moway.bytes_por_dir);
		if(!pData)
		{
			printf("No MEMORY");
			exit(-1);
		}
		length_mem = ((moway.reg_mem[i].Size + 1) * moway.bytes_por_dir);
		for(j=0; j<length_mem;j++)
		{
			pData[j]=255;
		}
		setDataMemoryRegion(i, pData);
	}

	moway.bytes_por_pack = rta.QueryResults.BytesPerPacket;

}


/******************  MOWAY USB functions ***********************************/

//********************************************************************
//	init  MOWAY DATA
//********************************************************************
int InitMOWAY()
{
	int i;
	moway.regiones=0;
	moway.bytes_por_dir =0;
	moway.reg_mem =(struct MEMORY_REGION*) malloc(MAX_DATA_REGIONS*sizeof(struct MEMORY_REGION));
	
	hsetup = LoadLibrary("setupapi.dll");
	huser = LoadLibrary("user32.dll");
	hHID = LoadLibrary("hid.dll");

	if (hsetup)
	{
		_SetupDiGetClassDevsUM = (SetupDiGetClassDevsUM)GetProcAddress(hsetup,"SetupDiGetClassDevsA");
		_SetupDiEnumDeviceInterfacesUM = (SetupDiEnumDeviceInterfacesUM)GetProcAddress(hsetup,"SetupDiEnumDeviceInterfaces");
		_SetupDiDestroyDeviceInfoListUM = (SetupDiDestroyDeviceInfoListUM)GetProcAddress(hsetup,"SetupDiDestroyDeviceInfoList");
		_SetupDiGetDeviceRegistryPropertyUM = (SetupDiGetDeviceRegistryPropertyUM)GetProcAddress(hsetup,"SetupDiGetDeviceRegistryPropertyA");
		_SetupDiEnumDeviceInfoUM = (SetupDiEnumDeviceInfoUM)GetProcAddress(hsetup,"SetupDiEnumDeviceInfo");
		_SetupDiGetDeviceInterfaceDetailUM = (SetupDiGetDeviceInterfaceDetailUM)GetProcAddress(hsetup,"SetupDiGetDeviceInterfaceDetailA");
	}
	if (huser)
	{
		_RegisterDeviceNotificationUM = (RegisterDeviceNotificationUM)GetProcAddress(huser,"RegisterDeviceNotificationA");
	}
	if (hHID)
	{
		_HidD_GetAttributesUM=(HidD_GetAttributesUM)GetProcAddress(hHID,"HidD_GetAttributes");
	}
	if ( _SetupDiGetClassDevsUM == NULL || _SetupDiEnumDeviceInterfacesUM == NULL
		|| _SetupDiDestroyDeviceInfoListUM == NULL || _SetupDiGetDeviceRegistryPropertyUM == NULL
		|| _SetupDiEnumDeviceInfoUM == NULL || _SetupDiGetDeviceInterfaceDetailUM == NULL
		|| _RegisterDeviceNotificationUM == NULL || _HidD_GetAttributesUM == NULL)
	{
		printf("Fail to load DLL");
		return (-1);
	}
  	if (moway.reg_mem!=NULL)
	{
		for (i=0;i<MAX_DATA_REGIONS;i++)
				setDataMemoryRegion(i,0);
		return 0;
	}
	return -1;
}
//********************************************************************
//	Open  MOWAY
//********************************************************************
int OpenMOWAY()
{
	PSP_DEVICE_INTERFACE_DETAIL_DATA detailData;
	HANDLE DeviceHandle;
	HANDLE hDevInfo;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	ULONG Length, Required;
	int MemberIndex = 0, Result;
	boolean LastDevice = FALSE, mowayDetected = FALSE;
	HIDD_ATTRIBUTES Attributes;
	DWORD ErrorStatusWrite = ERROR_SUCCESS; 
	DWORD ErrorStatusRead = ERROR_SUCCESS; 

	hDevInfo=_SetupDiGetClassDevsUM(&InterfaceClassGuid,NULL,NULL,DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
	devInfoData.cbSize = sizeof(devInfoData);
	MemberIndex = 0;
	LastDevice = FALSE;
	do
	{
		Result=_SetupDiEnumDeviceInterfacesUM(hDevInfo, NULL, &InterfaceClassGuid, MemberIndex, &devInfoData);
		if (Result != 0)
		{
			Result = _SetupDiGetDeviceInterfaceDetailUM(hDevInfo, &devInfoData, NULL, 0, &Length, NULL);
			//Allocate memory for the hDevInfo structure, using the returned Length.
			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
			//Set cbSize in the detailData structure.
			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			//Call the function again, this time passing it the returned buffer size.
			Result = _SetupDiGetDeviceInterfaceDetailUM(hDevInfo, &devInfoData, detailData, Length,&Required, NULL);
			// Open a handle to the device.
			DeviceHandle=CreateFile(detailData->DevicePath,0, FILE_SHARE_READ|FILE_SHARE_WRITE,
				(LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING, 0, NULL);
			
			Attributes.Size = sizeof(Attributes);
			Result = _HidD_GetAttributesUM(DeviceHandle,&Attributes);
			if (Attributes.VendorID == VENDOR_ID)
			{
				if (Attributes.ProductID == PRODUCT_ID)
				{
					mowayDetected=TRUE;
					moway.WriteHandleRFUSB = CreateFile(detailData->DevicePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,0, 0);
					ErrorStatusWrite = GetLastError();
					moway.ReadHandleRFUSB = CreateFile(detailData->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
					ErrorStatusRead = GetLastError();
					if (ErrorStatusRead != ERROR_SUCCESS && ErrorStatusWrite != ERROR_SUCCESS)
					{
						return FALSE;
					}
				}
				else
					CloseHandle(DeviceHandle);
			}
			else
				CloseHandle(DeviceHandle);
		}
		else
			LastDevice = TRUE;
		MemberIndex = MemberIndex + 1;
	}
	while ((LastDevice == FALSE) && (mowayDetected == FALSE));

	return mowayDetected;
}

//********************************************************************
//	Close  MOWAY
//********************************************************************
int CloseMOWAY()
{
	int ret=0;

	CloseHandle(moway.WriteHandleRFUSB);
	CloseHandle(moway.ReadHandleRFUSB);

	return ret;
}

//********************************************************************
//	Read from MOWAY
//********************************************************************
int ReadDataMOWAY(BOOTLOADER_COMMAND *data)
{
	int ret = 0;
	DWORD bytes_read;
	ret =ReadFile(moway.ReadHandleRFUSB, (char *)data->RawData, 65, &bytes_read,  0);
	return (int)bytes_read;
}

//********************************************************************
//	Write to MOWAY
//********************************************************************
int WriteDataMOWAY(BOOTLOADER_COMMAND data)
{
	int ret = 0;
	DWORD bytes_write;
	ret=WriteFile(moway.WriteHandleRFUSB, (char *)data.RawData, 65, &bytes_write,0);
	return (int)bytes_write;
}

/****************** END MOWAY USB functions ***********************************/


//********************************************************************
//	delay_sec
//********************************************************************
void delay_sec(int n)
{
	clock_t t = clock () + n * CLOCKS_PER_SEC;
	while(clock() < t)
		;
}


//********************************************************************
//	HexToInt
//********************************************************************
uint32_t HexToInt(char* s)
{
	uint32_t h = 0, k = 1,i;
	uint32_t len = strlen(s);
	for (i=0; i<len; i++) {
		char c = s[len-i-1];
		if ((c >= 'A') && (c <= 'F'))
			c -= 'A' - 10;
		else if ((c >= 'a') && (c <= 'f'))
			c -= 'a' - 10;
		else if((c >= '0') && (c <= '9'))
			c -= '0';
		else
			printf("Coversion HexToInt ERROR");
		h += c * k;
		k *= 16;
	}
	return h;
}

char* substr(char* cadena, int comienzo, int longitud)
{
	char *nuevo;

	if (longitud == 0) longitud = strlen(cadena)-comienzo-1;

	nuevo = (char*)malloc(sizeof(char) * (longitud+1));

	strncpy(nuevo, cadena + comienzo, longitud);

	nuevo[longitud]='\0';

	return nuevo;
}


/****************** Memory Region functions ***********************************/

//********************************************************************
//	setDataMemoryRegion
//********************************************************************
void setDataMemoryRegion(uint8_t region, uint8_t* p)
{
	if(region < 6)
		moway.vData[region] = p;
}

//********************************************************************
//	getDataMemoryRegion
//********************************************************************
uint8_t *getDataMemoryRegion(uint8_t region)
{
	return (region < 6)? moway.vData[region]: 0;
}

/****************** END ofMemory Region functions ***********************************/


