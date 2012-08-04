/*
 * cmd_def.h
 *
 *  Created on: Aug 2, 2012
 *      Author: sszilvasi
 */

#ifndef CMD_DEF_H_
#define CMD_DEF_H_


#include <string.h>
#include <stdlib.h>

#include "yellowstone.h"
#include "teton.h"
#include "joshua.h"

typedef struct _CMD_Type
{
	const char *CmdString;
	uint32_t (*CmdFunction)(uint32_t argc, char** argv);
} CMD_Type;


extern CMD_Type CMD_List[];

// General commands
uint32_t CmdHelp(uint32_t argc, char** argv);
uint32_t CmdLed(uint32_t argc, char** argv);
uint32_t CmdAfe(uint32_t argc, char** argv);

// MAX 2830 commands
uint32_t CmdReg(uint32_t argc, char** argv);
uint32_t CmdFreq(uint32_t argc, char** argv);
uint32_t CmdGain(uint32_t argc, char** argv);
uint32_t CmdLpf(uint32_t argc, char** argv);
uint32_t CmdMode(uint32_t argc, char** argv);
uint32_t CmdRssi(uint32_t argc, char** argv);
uint32_t CmdPa(uint32_t argc, char** argv);
//uint32_t CmdCal(uint32_t argc, char** argv);



#endif /* CMD_DEF_H_ */
