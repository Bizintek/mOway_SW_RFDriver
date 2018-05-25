//============================================================================
// Name        : mowayscratch.cpp
// Author      : Minirobots S.L.
// Version     : 0.1
// Copyright   :
// Description : Moway Robot and Scratch Communication Software
//============================================================================
#include "datatype.h"
#include "mowayRFUSB.h"
#include "bootloaderMOWAY.h"
#include "libmoway.h"


HANDLE thread_rfusb_read;
HANDLE thread_rfusb_send;
HANDLE thread_rfusb_timer;
HANDLE thread_motend_timer;


struct rfusb rf;
struct moway_sensor mow_sensor;
struct usbmoway moway;
uint8_t moway_command[8];
int timeout;
int timeout_motend;
int moway_receive;


//********************************************************************
//	usbinit_moway
//********************************************************************
void usbinit_moway()
{
	usb_init();

}

//********************************************************************
//	Exit Moway
//********************************************************************
void exit_moway()
{
	
}

//********************************************************************
//	Command Moway
//********************************************************************
int command_moway(uint8_t command, int waitcommand)
{
	int temp;
	static struct mowaysenddata old_data;

	if (rf.state!= RUNNING)
		return -1;
	memcpy(&old_data,&(rf.mowaydata[rf.datain]),sizeof(struct mowaysenddata));
	temp= rf.datain+1;
	if (temp>=MAX_BUFFER)
		temp=0;
	rf.mowaydata[temp].command=command;
	rf.mowaydata[temp].speed=rf.actual_data.speed;
	rf.mowaydata[temp].rotation=rf.actual_data.rotation;
	rf.mowaydata[temp].time=rf.actual_data.time;
	rf.mowaydata[temp].distance=rf.actual_data.distance;
	rf.mowaydata[temp].rotation_axis=rf.actual_data.rotation_axis;
	rf.mowaydata[temp].radius=rf.actual_data.radius;
	rf.mowaydata[temp].frequency=rf.actual_data.frequency;
	rf.mowaydata[temp].address=MOWAY_ADRESS;

	if (memcmp(&old_data,&(rf.mowaydata[temp]),sizeof(struct mowaysenddata))==0 && timeout==1)
	{
		return 1;
	}
	else
	{
		timeout=1;
		rf.datain=temp;
		if (waitcommand > 0)
		{
			while (waitcommand > 0 &&  rf.dataout < temp)
			{
				Sleep(20);
				waitcommand= waitcommand -20;
			}
			if ( waitcommand <= 0 )
			{
				return 2;
			}
		}
	}
	return 0;
}

//********************************************************************
//	Init Moway
//********************************************************************
int init_moway(uint8_t channel)
{
	rf.datain=0;
	rf.dataout=0;
	rf.actual_data.speed=50;
	rf.actual_data.rotation=25;
	rf.actual_data.radius=30;
	rf.actual_data.distance=0;
	rf.actual_data.time=0;
	rf.actual_data.rotation_axis=1;
	rf.actual_data.frequency=50;

	if (rf.state == RUNNING)
	{
		rf.state=STOPPED;
		CloseRFUSB();
	}

	rf.state=STOPPED;
	Sleep(50);
	if (  InitializeMoway(channel) > 0  )
	{
		rf.state=RUNNING;
		thread_rfusb_read = CreateThread(NULL, 0, read_rfusb_data, NULL, 0, NULL);
		if (thread_rfusb_read == NULL)
		{
			rf.state=STOPPED;
			exit(-1);
		}
		thread_rfusb_send = CreateThread(NULL, 0, send_rfusb_data, NULL, 0, NULL);
		if (thread_rfusb_send == NULL)
		{
			rf.state=STOPPED;
			exit(-1);
		}
		thread_rfusb_timer = CreateThread(NULL, 0, timer_rfusb, NULL, 0, NULL);
		if (thread_rfusb_timer == NULL)
		{
			rf.state=STOPPED;
			exit(-1);
		}
		thread_motend_timer =  CreateThread(NULL, 0, timer_motend, NULL, 0, NULL);
		if (thread_motend_timer == NULL)
		{
			rf.state=STOPPED;
			exit(-1);
		}
		command_moway(CMD_STOP,0);
		Sleep(20);
		command_moway(CMD_LEDSOFF,0);
		Sleep(20);
		command_moway(CMD_BUZZEROFF,0);
		Sleep(20);
		command_moway(CMD_RESET_DIST,0);
		Sleep(150);
		return 0;
	}
	return -1;
}

//********************************************************************
//	Close Moway
//********************************************************************
void close_moway()
{
	command_moway(CMD_STOP,0);
	Sleep(20);
	command_moway(CMD_LEDSOFF,0);
	Sleep(20);
	command_moway(CMD_BUZZEROFF,0);
	Sleep(50);
	rf.state=STOPPED;
	CloseRFUSB();
}



//********************************************************************
//	Set Speed
//********************************************************************
void set_speed(int speed)
{
	if (speed < 0)
		speed=10;
	if(speed>100)
		speed=100;
	rf.actual_data.speed=(uint8_t)speed;
}

//********************************************************************
//	Set Rotation
//********************************************************************
void set_rotation(int rotation)
{
	rotation=(int)(rotation/3.6);
	if (rotation < 0)
		rotation=0;
	if(rotation>100)
		rotation=100;
	rf.actual_data.rotation=(uint8_t)rotation;
}

//********************************************************************
//	Set Distance
//********************************************************************
void set_distance(uint8_t distance)
{
	rf.actual_data.distance=(uint8_t)distance;
}

//********************************************************************
//	Set Time
//********************************************************************
void set_time(uint8_t time)
{
	rf.actual_data.time=(uint8_t)time;
}

//********************************************************************
//	Set Radius
//********************************************************************
void set_radius(uint8_t radius)
{
	if (radius < 0)
		radius=0;
	if(radius>100)
		radius=100;
	rf.actual_data.radius=(uint8_t)radius;
}

//********************************************************************
//	Set Frequency
//********************************************************************
void set_frequency (int frequency)
{
	if (frequency!=0)
		frequency = 4000000 / (64 * frequency) - 1;
	if (frequency < 0)
		frequency=0;
	if(frequency>255)
		frequency=255;
	rf.actual_data.frequency=(uint8_t)frequency;
}

//********************************************************************
//	Set Rotation Axis
//********************************************************************
void set_rotation_axis(uint8_t axis)
{
	if (axis!=0)
		rf.actual_data.rotation_axis=1;
	else
		rf.actual_data.rotation_axis=0;
}


//********************************************************************
//	Obstacle Sensor
//********************************************************************
int get_obs_side_left()
{
	return mow_sensor.obstacle[0];
}
int get_obs_center_left()
{
	return mow_sensor.obstacle[1];
}
int get_obs_center_right()
{
	return mow_sensor.obstacle[2];
}
int get_obs_side_right()
{
	return mow_sensor.obstacle[3];
}

//********************************************************************
//	Line Sensor
//********************************************************************
int get_line_left()
{
	return mow_sensor.line[0];
}
int get_line_right()
{
	return mow_sensor.line[1];
}

//********************************************************************
//	Accelerometer
//********************************************************************
float get_accel_X()
{
	return mow_sensor.accel[0];
}
float get_accel_Y()
{
	return mow_sensor.accel[1];
}
float get_accel_Z()
{
	return mow_sensor.accel[2];
}

//********************************************************************
//	Mic
//********************************************************************
int get_mic()
{
	return mow_sensor.mic;
}

//********************************************************************
//	Mot End
//********************************************************************
int get_mot_end()
{
	return mow_sensor.mot_end;
}

//********************************************************************
//	Light
//********************************************************************
int get_light()
{
	return mow_sensor.light;
}

//********************************************************************
//	Distance
//********************************************************************
int get_distance()
{
	return mow_sensor.distance;
}

//********************************************************************
//      Wait for MOT END
//********************************************************************
int wait_mot_end(int time)
{
        timeout_motend=1000;
        while (mow_sensor.mot_end==1 && timeout_motend>0)
                Sleep(10);
        if (timeout_motend<=0)
                return -1;
        if (time==0)
                time=26000;
		else
			timeout_motend=time*1000;
        while (mow_sensor.mot_end == 0 && timeout_motend>0)
                Sleep(10);
        if (timeout_motend<=0)
                return -2;
		timeout_motend=0;
        return 0;
}

int moway_active()
{
	return moway_receive;
}


/******* MOWAY ROBOT functions *********/


//********************************************************************
//      Init Prog Moway
//********************************************************************
int init_prog_moway()
{
	return InitMOWAY();
}

//********************************************************************
//      Program Moway with hex file
//********************************************************************
int program_moway(char * file)
{
	int ret=-1;
	if (OpenMOWAY() == TRUE)
	{
		InformationMOWAY();
		if (OpenHexFileMOWAY(file,200)==0)
		{
			EraseMOWAY();
			ProgramMOWAY();
			ret= 0;
		}
		else
		{
			ret=-2;
			printf("Hex File not found\n");
		}
		CloseMOWAY();
	}
	return ret;
}


//********************************************************************
//      Program Moway with hex file and Channel
//********************************************************************
int program_moway_channel(char * file, int channel)
{
	int ret=-1;
	if (OpenMOWAY() == TRUE)
	{
		InformationMOWAY();
		if (OpenHexFileMOWAY(file,channel)==0)
		{
			EraseMOWAY();
			ProgramMOWAY();
			ret= 0;
		}
		else
		{
			ret=-2;
			printf("Hex File not found\n");
		}
		CloseMOWAY();
	}
	return ret;
}


//********************************************************************
//     Read Moway Batt
//********************************************************************
int read_moway_batt ()
{
	int ret=-1;
	if (OpenMOWAY() == TRUE)
	{
		if (ReadBattAndFirmwareMOWAY()==0)
		{
			ret=moway.mowaybatt;
		}
		
		CloseMOWAY();
	}
	return ret;
}