/*****************************************************************************
*
* @file LuaVM.h
*
* This header contains constants and definitions for
* a small virtual machine based on the Lua vm.
*
* Copyright (c) 2007, 2008 Brandon Blodget
* Copyright (c) 2013 OLogic Inc.
* All rights reserved.
*
* Author: Brandon Blodget
*
*****************************************************************************/

#ifndef LUA_VM_H_
#define LUA_VM_H_

#include "global.h"

extern UINT16 luaVMPC;
extern UINT8 *luaVMProgramName;
extern UINT8 luaVMFunctionsIndex;
extern UINT32 luaVMCallStackIndex;

void luaVMInit(void);
void luaVMReset(void);
void luaVMRun(void);
void luaVMCont(void);
INT32 luaVMStep(BOOL print_op);

#endif
