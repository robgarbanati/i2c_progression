/*****************************************************************************
*
* @file LuaTables.c
*
* This file implements a small virtual machine
* based on the Lua VM.
*
* Copyright (c) 2007, 2008 Brandon Blodget
* Copyright (c) 2013 OLogic Inc.
* All rights reserved.
*
* Author: Brandon Blodget
*
*****************************************************************************/

#include "global.h"
#include "lua_vm.h"
#include "lua_tables.h"

// Define to control whether tables are held in global arrays.
#define LUA_VM_TABLES_AS_ARRAYS 0

// Global Lua VM tables.

// Intructions table.
#if LUA_VM_TABLES_AS_ARRAYS
UINT32 luaVMInstructionsTable[MAX_PROG_SIZE];
#endif
UINT32 luaVMInstructionsSize;
UINT32 *luaVMInstructions;

// Functions table.
#if LUA_VM_TABLES_AS_ARRAYS
LUA_VM_FUNCTION luaVMFunctionsTable[MAX_NUM_FUNCTIONS];
#endif
UINT32 luaVMFunctionsSize;
LUA_VM_FUNCTION *luaVMFunctions;

// Constants table.
#if LUA_VM_TABLES_AS_ARRAYS
LUA_VM_CONSTANT luaVMConstantsTable[MAX_CONST_SIZE];
#endif
UINT32 luaVMConstantsSize;
LUA_VM_CONSTANT *luaVMConstants;

// Strings table.
#if LUA_VM_TABLES_AS_ARRAYS
UINT8 luaVMStringsTable[MAX_STR_SIZE];
#endif
UINT32 luaVMStringsSize;
UINT8 *luaVMStrings;

// Globals table (signed).
#if LUA_VM_TABLES_AS_ARRAYS
INT32 luaVMGlobalsTable[MAX_GLOBAL_SIZE];
#endif
UINT32 luaVMGlobalsSize;
INT32 *luaVMGlobals;

// Registers table (signed).
#if LUA_VM_TABLES_AS_ARRAYS
INT32 luaVMRegistersTable[MAX_REGISTERS_SIZE];
#endif
UINT32 luaVMRegistersSize;
INT32 *luaVMRegisters;

// Call stack table.
#if LUA_VM_TABLES_AS_ARRAYS
LUA_VM_CALLSTACK luaVMCallStackTable[MAX_CALLSTACK_SIZE];
#endif
UINT32 luaVMCallStackSize;
LUA_VM_CALLSTACK *luaVMCallStack;

// Initialization of static in-memory tables.
void luaVMTablesInit(void)
{
#if LUA_VM_TABLES_AS_ARRAYS
	// Initialize tables kept global arrays.
	luaVMInstructions = luaVMInstructionsTable;
	luaVMFunctions = luaVMFunctionsTable;
	luaVMConstants = luaVMConstantsTable;
	luaVMStrings = luaVMStringsTable;
	luaVMGlobals = luaVMGlobalsTable;
	luaVMRegisters = luaVMRegistersTable;
	luaVMCallStack = luaVMCallStackTable;
#endif
}
