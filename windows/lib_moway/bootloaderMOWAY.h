/*
 * bootloaderMOWAY.h
 *
 *  Created on: 26/03/2013
 *      Author: Dani
 */

#ifndef BOOTLOADERMOWAY_H_
#define BOOTLOADERMOWAY_H_

#include "datatype.h"

#define DEVICE_ID  "Vid_04d8&Pid_003c"	

#define VENDOR_ID                       0x04D8
#define PRODUCT_ID                      0x003C

// Request values ​​of specific HID classes.
// See section 7.2 of the HID specification.
#define HID_GET_REPORT                  0x01
#define HID_GET_IDLE                    0x02
#define HID_GET_PROTOCOL                0x03
#define HID_SET_REPORT                  0x09
#define HID_SET_IDLE                    0x0A
#define HID_SET_PROTOCOL                0x0B
#define HID_REPORT_TYPE_INPUT           0x01
#define HID_REPORT_TYPE_OUTPUT          0x02
#define HID_REPORT_TYPE_FEATURE         0x03

// Commands for the bootloader
#define QUERY_DEVICE                    0x02
#define UNLOCK_CONFIG                   0x03
#define ERASE_DEVICE                    0x04
#define PROGRAM_DEVICE                  0x05
#define PROGRAM_COMPLETE                0x06
#define GET_DATA                        0x07
#define RESET_DEVICE                    0x08
#define MOW_BATT			0x09
#define GET_ENCRYPTED_FF                0xFF

// Results of a query(QUERY)
#define QUERY_IDLE                      0xFF
#define QUERY_RUNNING                   0x00
#define QUERY_SUCCESS                   0x01
#define QUERY_WRITE_FILE_FAILED         0x02
#define QUERY_READ_FILE_FAILED          0x03
#define QUERY_MALLOC_FAILED             0x04

// Results of a programming operation(PROGRAMMING)
#define PROGRAM_IDLE                    0xFF
#define PROGRAM_RUNNING                 0x00
#define PROGRAM_SUCCESS                 0x01
#define PROGRAM_WRITE_FILE_FAILED       0x02
#define PROGRAM_READ_FILE_FAILED        0x03
#define PROGRAM_RUNNING_ERASE           0x05
#define PROGRAM_RUNNING_PROGRAM         0x06

// Results of a delete operation(ERASE)
#define ERASE_IDLE                      0xFF
#define ERASE_RUNNING                   0x00
#define ERASE_SUCCESS                   0x01
#define ERASE_WRITE_FILE_FAILED         0x02
#define ERASE_READ_FILE_FAILED          0x03
#define ERASE_VERIFY_FAILURE            0x04
#define ERASE_POST_QUERY_FAILURE        0x05
#define ERASE_POST_QUERY_RUNNING        0x06
#define ERASE_POST_QUERY_SUCCESS        0x07

// Results of a verification operation(VERIFY)
#define VERIFY_IDLE                     0xFF
#define VERIFY_RUNNING                  0x00
#define VERIFY_SUCCESS                  0x01
#define VERIFY_WRITE_FILE_FAILED        0x02
#define VERIFY_READ_FILE_FAILED         0x03
#define VERIFY_MISMATCH_FAILURE         0x04

// Results of a reading operation(READ)
#define READ_IDLE                       0xFF
#define READ_RUNNING                    0x00
#define READ_SUCCESS                    0x01
#define READ_READ_FILE_FAILED           0x02
#define READ_WRITE_FILE_FAILED          0x03

// Unlock configuration results(UNLOCK CONFIG)
#define UNLOCK_CONFIG_IDLE              0xFF
#define UNLOCK_CONFIG_RUNNING           0x00
#define UNLOCK_CONFIG_SUCCESS           0x01
#define UNLOCK_CONFIG_FAILURE           0x02

// Bootloader states
#define BOOTLOADER_IDLE                 0xFF
#define BOOTLOADER_QUERY                0x00
#define BOOTLOADER_PROGRAM              0x01
#define BOOTLOADER_ERASE                0x02
#define BOOTLOADER_VERIFY               0x03
#define BOOTLOADER_READ                 0x04
#define BOOTLOADER_UNLOCK_CONFIG        0x05
#define BOOTLOADER_RESET                0x06

// Results of a reset(RESET)
#define RESET_IDLE                      0xFF
#define RESET_RUNNING                   0x00
#define RESET_SUCCESS                   0x01
#define RESET_WRITE_FILE_FAILED         0x02

// Types of memory regions
#define MEMORY_REGION_PROGRAM_MEM       0x01
#define MEMORY_REGION_EEDATA            0x02
#define MEMORY_REGION_CONFIG            0x03
#define MEMORY_REGION_END               0xFF

// HEX line types
#define HEX_FILE_EXTENDED_LINEAR_ADDRESS 0x04
#define HEX_FILE_EOF                     0x01
#define HEX_FILE_DATA                    0x00

// This is the number of bytes per line of the batery
#define BATT_FULL 255
#define BATT_MAX  418
#define BATT_MIN  370

// Number of bytes per line of the HEX file
#define HEX_FILE_BYTES_PER_LINE          16
#define MAX_LONG_LIN					 48

// Device families
#define DEVICE_FAMILY_PIC18              1
#define DEVICE_FAMILY_PIC24              2
#define DEVICE_FAMILY_PIC32              3

#define PIC24_RESET_REMAP_OFFSET         0x1400

#define CH_LINE							725
#define CH_POSITION						6


void ProgramMOWAY();

int InitMOWAY();
int OpenMOWAY();
int CloseMOWAY();
int EraseMOWAY();
void InformationMOWAY();
int ReadBattAndFirmwareMOWAY();
int WriteDataMOWAY(BOOTLOADER_COMMAND data);
int ReadDataMOWAY(BOOTLOADER_COMMAND *data);
int OpenHexFileMOWAY(const char *strData, uint8_t channel);

uint32_t HexToInt(char* s);
char* substr(char* cadena, int comienzo, int longitud);
uint8_t *getDataMemoryRegion(uint8_t region);
void setDataMemoryRegion(uint8_t region, uint8_t* p);
void delay_sec(int n);


#endif /* BOOTLOADERMOWAY_H_ */
