/*
 * mowayRFUSB.c
 *
 *  Created on: 18/03/2013
 *      Author: Dani
 */
#include "mowayRFUSB.h"
#include "datatype.h"

extern struct rfusb rf;
extern struct moway_sensor mow_sensor;
extern int timeout;
extern int timeout_motend;
extern int moway_receive;

/****************** RFUSB functions ***********************************/

//********************************************************************
//	InitializeMoway
//********************************************************************
int InitializeMoway(uint8_t channel)
{
	int ret;
	uint8_t msgData[10];
	
	struct usb_bus *UsbBus= NULL;
	struct usb_device *UsbDevice = NULL; 
	
	usb_find_busses ();
    usb_find_devices ();

	rf.vid = VENDOR_ID_RFUSB;
	rf.pid = PRODUCT_ID_RFUSB;

	for (UsbBus = usb_get_busses(); UsbBus; UsbBus = UsbBus->next) {
            for (UsbDevice = UsbBus->devices; UsbDevice; UsbDevice = UsbDevice->next) {
                    if (UsbDevice->descriptor.idVendor == VENDOR_ID_RFUSB && UsbDevice->descriptor.idProduct== PRODUCT_ID_RFUSB) {
                            break;
                    }
            }
    }
    if (!UsbDevice) 
		return -1;
	rf.devh = usb_open (UsbDevice);

    if (usb_set_configuration (rf.devh, 1) < 0) {
            usb_close (rf.devh);
            return -1;
    }

    if (usb_claim_interface (rf.devh, 0) < 0) {
            usb_close (rf.devh);
            return -1;
    }

	msgData[0]=CMD_WAIT_APP;
	msgData[1]=0;
	ret=usb_bulk_write(rf.devh, ENDPOINT_INT_OUT, (char*)msgData, sizeof(msgData), TIMEOUT);

	if (ret<0)
	{
		usb_release_interface(rf.devh, 0);
		usb_reset(rf.devh);
		usb_close(rf.devh);
		return -1;
	}
	Sleep(15);

	msgData[0]=CMD_RF_ON;
	msgData[1]=channel;//channel
	msgData[2]=RFUSB_ADDRESS;//RFUSB_ADDRESS

	ret=usb_bulk_write(rf.devh, ENDPOINT_INT_OUT, (char*) msgData, sizeof(msgData), TIMEOUT);
	if (ret<0)
	{
		usb_release_interface(rf.devh, 0);
		usb_reset(rf.devh);
		usb_close(rf.devh);
		return -1;
	}
	Sleep(15);
	return ret;
}


//********************************************************************
//	CloseMoway
//********************************************************************
int CloseRFUSB()
{
	int ret;
	uint8_t msgData[10];

	msgData[0]=CMD_CLOSE;
	msgData[1]=0;
	ret=usb_bulk_write(rf.devh, ENDPOINT_INT_OUT, (char*)msgData, sizeof(msgData), TIMEOUT);

	Sleep(100);

	usb_release_interface(rf.devh, 0);
	usb_close(rf.devh);

	return ret;
}

//********************************************************************
//	Read from RFUSB
//********************************************************************

int ReadDataRFUSB(uint8_t *data)
{
	int ret = 0;
	ret = usb_bulk_read(rf.devh, ENDPOINT_INT_IN, (char*)data, 64, TIMEOUT);
	if (ret >0)
	{
		return ret;
	}
	else if (ret == -116)
	{
		return 0;
	}
	else
	{ 
		return ret;
	}
}


//********************************************************************
//	Send data RFUSB
//********************************************************************

int SendDataRFUSB(struct mowaysenddata data)
{
	int ret;
	uint8_t msgData[10];

	msgData[0] = CMD_SEND_RF;
	msgData[1] = MOWAY_ADRESS;
	msgData[2] = data.command;
	msgData[3] = data.speed;
	msgData[4] = data.rotation;
	msgData[5] = data.time;
	msgData[6] = data.distance;
	msgData[7] = data.rotation_axis;
	msgData[8] = data.radius;
	msgData[9] = data.frequency;

	ret=usb_bulk_write(rf.devh, ENDPOINT_INT_OUT, msgData, 10,TIMEOUT);
	Sleep(15);
	return ret;
}


/****************** END of RFUSB functions ***********************************/


int DataManageMoway(uint8_t *a)
{
	int n=0;

	static uint16_t tmpMic1,tmpMic2,tmpMic3;

	if (a[CMD_POS] == CMD_FRAME_01)
	{
		mow_sensor.obstacle[0] = (uint16_t)(a[CMD_DATA_0] * 0.39); //OBS_SIDE_LEFT
		mow_sensor.obstacle[1] = (uint16_t)(a[CMD_DATA_0 + 1] * 0.39); //OBS_CENTER_LEFT
		mow_sensor.obstacle[2] = (uint16_t)(a[CMD_DATA_0 + 2] * 0.39); //OBS_CENTER_RIGHT
		mow_sensor.obstacle[3] = (uint16_t)(a[CMD_DATA_0 + 3] * 0.39); //OBS_SIDE_RIGHT
		mow_sensor.line[1] = (uint16_t)(a[CMD_DATA_0 + 4] * 0.39); //SEN_LINE_RIGHT
		mow_sensor.line[0] = (uint16_t)(a[CMD_DATA_0 + 5] * 0.39); //SEN_LINE_LEFT
		mow_sensor.mot_end = (uint16_t)a[CMD_DATA_0 + 6]; //MOT_END
	}
	else if (a[CMD_POS] == CMD_FRAME_02)
	{

		mow_sensor.distance = (uint16_t)a[CMD_DATA_0 + 3] * 255 + a[CMD_DATA_0 + 4]; //MOT_DISTANCE
		mow_sensor.light = (uint16_t)a[CMD_DATA_0 + 5]; //SEN_LIGHT
		tmpMic1 = (uint16_t)a[CMD_DATA_0 + 6];
		mow_sensor.mic = (uint16_t)( (tmpMic1 + tmpMic2 + tmpMic3)*0.39/3);
		tmpMic3 = tmpMic2;
		tmpMic2 = tmpMic1;
		mow_sensor.accel[0] = (float) (a[CMD_DATA_0] - 128) *(float) 0.016;
		mow_sensor.accel[1] = (float) (a[CMD_DATA_0 + 1] - 128) *(float) 0.016;
		mow_sensor.accel[2] = (float) (a[CMD_DATA_0 + 2] - 128) *(float) 0.016;
	}
	moway_receive++;
	if (moway_receive>2)
		moway_receive=2;
	return n;
}


//********************************************************************
//	Thread: Moway Robot data read
//********************************************************************
DWORD  WINAPI read_rfusb_data(LPVOID data)
{
   int ret=0,i;
   uint8_t RawDataIn[64];

   while (rf.state==RUNNING)
   {
	   ret=ReadDataRFUSB( RawDataIn);

	   if (ret<0 && rf.state==RUNNING)
	   {
		   printf("Error reading RFUSB\n");
		   rf.state=STOPPED;
		   //SEND SIGNAL DISCONNECT
		   return (-1);
	   }
	   else if (ret == 2)
	   {
		   if (RawDataIn[0]== CMD_SEND_RF)
		   {
			   if (RawDataIn[1]== 0)
			   {
				   rf.dataout++;
				   if (rf.dataout>=MAX_BUFFER)
					   rf.dataout=0;
			   }
		   }
	   }
	   else if (ret%9==0)
	   {
		   for(i=0;i<ret;i+=9)
		   {
			   DataManageMoway(&(RawDataIn[i]));

		   }
	   }
   }
   return 0;
}


//********************************************************************
//	Thread: Moway Robot data send
//********************************************************************
DWORD  WINAPI send_rfusb_data(LPVOID data)
{
   static uint8_t temp,ret;

   while (rf.state == RUNNING)
   {
	   if (rf.datain != rf.dataout)
	   {
		   temp=rf.dataout+1;
		   if (temp>=MAX_BUFFER)
			   temp=0;
		   ret=SendDataRFUSB(rf.mowaydata[temp]);
	   }
	   else
	   {
		   Sleep(25);
	   }
   }
   return 0;
}

//********************************************************************
//	Thread: Timer
//********************************************************************
DWORD  WINAPI timer_rfusb(LPVOID param)
{
	while (rf.state == RUNNING)
	{
		if (timeout == 1)
		{
			Sleep(200);
			timeout=0;
		}
		else
		{
		   Sleep(200);
		}
		moway_receive--;
		if (moway_receive < 0)
			moway_receive=0;
	}
	return 0;
}

//********************************************************************
//	Thread: Timer MOT_END
//********************************************************************
DWORD  WINAPI timer_motend(LPVOID data)
{
	int temp;

	while (rf.state == RUNNING)
	{
		if (timeout_motend > 0 )
		{
			temp=timeout_motend;
			while (temp >0 && timeout_motend >0)
			{
				Sleep(100);
				temp=temp-100;
			}
		}
		Sleep(50);
	}
	return 0;
}

