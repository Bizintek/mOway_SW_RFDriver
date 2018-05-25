/*
 * mowayRFUSB.h
 *
 *  Created on: 18/03/2013
 *      Author: Dani
 */

#ifndef MOWAYRFUSB_H_
#define MOWAYRFUSB_H_
#include "datatype.h"


//********************************************************************
//	Constants
//********************************************************************

/* Identificacion del HID por VID-PID */

#define VENDOR_ID_RFUSB                 0x04D8
#define PRODUCT_ID_RFUSB                0x0014

/* state en struct rfusb */
#define STOPPED							0
#define RUNNING							1

//Movement Commands
#define CMD_GO              0xE1
#define CMD_BACK            0xE2
#define CMD_GOLEFT          0xE3
#define CMD_GORIGHT         0xE4
#define CMD_BACKLEFT        0xE6//0xE5
#define CMD_BACKRIGHT  		0xE5//0xE6
#define CMD_STOP            0xE7
#define CMD_ROTATELEFT      0xE8
#define CMD_ROTATERIGHT     0xE9
#define CMD_GO_SIMPLE       0xEA
#define CMD_BACK_SIMPLE     0xEB
#define CMD_LEFT_SIMPLE     0xEC
#define CMD_RIGHT_SIMPLE    0xED
#define CMD_TURN_AROUND     0xEE
#define CMD_RESET_DIST      0xEF

//Leds Commands
#define CMD_FRONTLEDON      0xA0
#define CMD_FRONTLEDOFF     0xA4
#define CMD_GREENLEDON      0xA2
#define CMD_GREENLEDOFF     0xA6
#define CMD_BRAKELEDON      0xA1
#define CMD_BRAKELEDOFF     0xA5
#define CMD_REDLEDON        0xA3
#define CMD_REDLEDOFF       0xA7
#define CMD_FRONTBLINK      0xA8
#define CMD_BRAKEBLINK      0xA9
#define CMD_GREENBLINK      0xAA
#define CMD_REDBLINK        0xAB
#define CMD_LEDSON          0xAC
#define CMD_LEDSOFF         0xAD
#define CMD_LEDSBLINK       0xAE

//Sound Commands
#define CMD_BUZZERON        0xC0
#define CMD_BUZZEROFF       0xC1
#define CMD_MELODYCHARGE    0xC2
#define CMD_MELODYFAIL      0xC3

//Complex Commands
#define CMD_LINE_FOLLOW_L   0x91
#define CMD_LINE_FOLLOW_R   0x92
#define CMD_ENCLOSED        0x93
#define CMD_DEFENDER        0x94
#define CMD_PUSH            0x95
#define CMD_PUSH2           0x96

#define WHEEL				0x00
#define CENTER				0x01

#define CMD_CONFIGURACION  	0xC0
#define CMD_ATTACHINFO  	0xAA
#define CMD_SEND_RF  		0xFF
#define CMD_DATA_RECEIVED  	0x01
#define CMD_WAIT_APP  0xCF
#define CMD_FRAME_01  		0xB5
#define CMD_FRAME_02  		0xB6
#define CMD_FRAME_03  		0xB7
#define CMD_FRAME_LENGTH  	9
#define CMD_SEND_LENGTH  	2
#define CMD_CONFIG_LENGTH  	12
#define CMD_POS  			8
#define CMD_DATA_0  		1

#define MAX_DATA 			512
#define VAR_LIMIT			20
#define MAX_TX_COUNTER		4
#define MAX_SOCKET_DATA		1024
#define MAX_VAR_NAME		1024

// Comandos RF
#define CMD_WAIT_APP			0xCF
#define CMD_RF_ON			0xAA
#define CMD_CLOSE			0xBB
#define CMD_SEND_RF			0xFF
#define RFUSB_ADDRESS					1
#define MOWAY_ADRESS		2



//********************************************************************
//	Functions
//********************************************************************
int InitializeMoway(uint8_t channel);
int CloseRFUSB();
int  ReadDataRFUSB(uint8_t *data);
int SendDataRFUSB(struct mowaysenddata data);
int DataManageMoway(uint8_t *a);
DWORD  WINAPI read_rfusb_data(LPVOID data);
DWORD  WINAPI send_rfusb_data(LPVOID data);
DWORD  WINAPI timer_rfusb(LPVOID param);
DWORD  WINAPI timer_motend(LPVOID data);


#endif /* MOWAYRFUSB_H_ */
