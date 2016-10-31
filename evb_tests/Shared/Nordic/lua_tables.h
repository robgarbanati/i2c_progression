/*****************************************************************************
*
* @file LuaTables.h
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

#ifndef LUA_TABLES_H_
#define LUA_TABLES_H_

#include "global.h"

// Lua VM constant table entry structure.
typedef struct
{
	INT32 constant;  		// signed
	UINT8 type;
	UINT8 globalIndex;
} LUA_VM_CONSTANT;

// Lua VM function table entry structure.
typedef struct
{
	UINT16 codeStart;
	UINT16 codeSize;
	UINT8 stackStart;
	UINT8 stackSize;
	UINT8 constStart;
	UINT8 constSize;
} LUA_VM_FUNCTION;

// Lua VM call stack table entry structure.
typedef struct
{
	UINT16 pc;  				// the program counter
	UINT8 fpi;  				// the function prototype number
	UINT8 retReg;  			// the register to start putting return values into
} LUA_VM_CALLSTACK;

// Global table sizes
#define MAX_PROG_SIZE 1024
#define MAX_CONST_SIZE 128
#define MAX_STR_SIZE 1024
#define MAX_GLOBAL_SIZE 216
#define MAX_NUM_FUNCTIONS 15
#define MAX_CALLSTACK_SIZE 20
#define MAX_REGISTERS_SIZE 256
#define MAX_PARAMS 10

// Program instruction table.
extern UINT32 luaVMInstructionsSize;
extern UINT32 *luaVMInstructions;

// Constant storage table.
extern UINT32 luaVMConstantsSize;
extern LUA_VM_CONSTANT *luaVMConstants;

// Area to store function info.
extern UINT32 luaVMFunctionsSize;
extern LUA_VM_FUNCTION *luaVMFunctions;

// Area to store string constants.
extern UINT32 luaVMStringsSize;
extern UINT8 *luaVMStrings;

// Area to store globals (signed).
extern UINT32 luaVMGlobalsSize;
extern INT32 *luaVMGlobals;

// The call stack.
extern LUA_VM_CALLSTACK *luaVMCallStack;
extern UINT32 luaVMCallStackSize;
extern UINT32 luaVMCallStackIndex;

// All the working registers (signed).
extern INT32 *luaVMRegisters;
extern UINT32 luaVMRegistersSize;

// Table initialization.
void luaVMTablesInit(void);

#endif
