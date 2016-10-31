/*****************************************************************************
*
* @file LuaLib.h
*
* This header contains constants and definitions for
* functions that can be called from lua
*
* Copyright (c) 2007, 2008 Brandon Blodget
* Copyright (c) 2013 OLogic Inc.
* All rights reserved.
*
* Author: Brandon Blodget
*
*****************************************************************************/

#ifndef LUA_LIB_H_
#define LUA_LIB_H_

#include "global.h"

// List of all the library functions.
enum luaCFunctions {
	LUA_SYS_ZERO,
	LUA_SYS_PRINT_NUM,		// 1
	LUA_SYS_PRINT_STR,		// 2
	LUA_SYS_SET_LED,			// 3
	LUA_SYS_GET_BUTTON,		// 4
	LUA_SYS_DELAY,				// 5
	LUA_SYS_END						// 6
};

void luaSysDelay(UINT32 delay);
void luaSysSetLED(UINT8 led_num, UINT8 state);
UINT32 luaSysGetButton(void);

#endif
