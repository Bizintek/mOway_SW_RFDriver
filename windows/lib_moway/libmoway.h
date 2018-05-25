/*
 * libmoway.h
 *
 *  Created on: 10/04/2013
 *      Author: Dani
 */

#ifndef LIBMOWAY_H_
#define LIBMOWAY_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


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

__declspec(dllexport) void usbinit_moway();
__declspec(dllexport) void exit_moway();
__declspec(dllexport) int init_moway(uint8_t channel);
__declspec(dllexport) void close_moway();
__declspec(dllexport) int command_moway(uint8_t command, int waitcommand);

__declspec(dllexport) void set_speed(int speed);
__declspec(dllexport) void set_rotation(int rotation);
__declspec(dllexport) void set_distance(uint8_t distance);
__declspec(dllexport) void set_time(uint8_t time);
__declspec(dllexport) void set_radius(uint8_t radius);
__declspec(dllexport) void set_frequency (int frequency);
__declspec(dllexport) void set_rotation_axis(uint8_t axis);

__declspec(dllexport) int get_obs_side_left();
__declspec(dllexport)int get_obs_center_left();
__declspec(dllexport) int get_obs_center_right();
__declspec(dllexport) int get_obs_side_right();
__declspec(dllexport) int get_line_left();
__declspec(dllexport) int get_line_right();
__declspec(dllexport) int get_mic();
__declspec(dllexport) int get_mot_end();
__declspec(dllexport) int get_light();
__declspec(dllexport) int get_distance();
__declspec(dllexport) float get_accel_X();
__declspec(dllexport) float get_accel_Y();
__declspec(dllexport) float get_accel_Z();
__declspec(dllexport) int wait_mot_end(int time);
__declspec(dllexport) int moway_active();


__declspec(dllexport) int init_prog_moway();
__declspec(dllexport) int program_moway(char * file);
__declspec(dllexport) int program_moway_channel(char * file, int channel);
__declspec(dllexport) int read_moway_batt ();





#endif /* LIBMOWAY_H_ */
