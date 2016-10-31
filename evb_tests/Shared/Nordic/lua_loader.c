/*****************************************************************************
*
* @file luaLib.c
*
* This file implements C functions that can be
* called from a Lua script. 
*
* Copyright (c) 2007, 2008 Brandon Blodget
* Copyright (c) 2013 OLogic Inc.
* All rights reserved.
*
* Author: Brandon Blodget
*
*****************************************************************************/

#include <string.h>
#include "global.h"
#include "lua_vm.h"
#include "lua_loader.h"
#include "lua_tables.h"
#include "xprintf.h"

// Lua image header.
typedef struct
{
	// Lua standard header.
	UINT8 signature[4];
	UINT8 version;
	UINT8 format;
	UINT8 endian;
	UINT8 intSize;
	UINT8 sizeSize;
	UINT8 sizeInstruction;
	UINT8 sizeNumber;
	UINT8 integral;

	// Lua table sizes.
	UINT32 instructionsSize;
	UINT32 functionsSize;
	UINT32 constantsSize;
	UINT32 stringsSize;
	UINT32 globalsSize;
} luaVMImageHeader;

// Memory space for Lua image.
__align(4) UINT8 luaVMImage[LUA_VM_IMAGE_MAX_SIZE];

static UINT32 fixupSize(UINT32 size)
{
	// Adjust the table size to be a multiple of four.
	if ((size & 0x3) != 0) size = ((size >> 2) + 1) << 2;
	return size;
}

// The first 12 bytes of every Lua image file should have the following signature.
static const UINT8 signature[12] = { 0xff, 0x4c, 0x75, 0x61, 0x51, 0x00, 0x01, 0x04, 0x04, 0x04, 0x04, 0x01 };

// Loads the image into memory.
BOOL luaVMImageLoad(SPIFSFile *luaFile)
{
	UINT32 readSize;
	UINT32 imageSize;
	luaVMImageHeader header;

	// Initialize the virtual machine tables.
	luaVMTablesInit();

	// Read the Lua image header from the file.
	if (spifs_read(luaFile, (UINT8 *) &header, sizeof(header)) < sizeof(header)) return FALSE;

	// Validate the first 12 bytes of the Lua image file header.
	if (memcmp(&header, &signature, sizeof(signature)) != 0) return FALSE;
	
	// Set the tables sizes from the header.
	luaVMInstructionsSize = header.instructionsSize;
	luaVMConstantsSize = header.constantsSize;
	luaVMFunctionsSize = header.functionsSize;
	luaVMStringsSize = header.stringsSize;
	luaVMGlobalsSize = header.globalsSize;
	luaVMCallStackSize = MAX_CALLSTACK_SIZE;
	luaVMRegistersSize = MAX_REGISTERS_SIZE;

	// Set the positions of the table pointers for the Lua sections 
	// that will be read from the file system. Notice that we fixup 
	// the length of each section to be an even multiple of 4 bytes.
	imageSize = 0;
	luaVMInstructions = (UINT32 *) &luaVMImage[imageSize];
	imageSize += fixupSize(luaVMInstructionsSize * sizeof(UINT32));
	luaVMFunctions = (LUA_VM_FUNCTION *) &luaVMImage[imageSize];
	imageSize += fixupSize(luaVMFunctionsSize * sizeof(LUA_VM_FUNCTION));
	luaVMConstants = (LUA_VM_CONSTANT *) &luaVMImage[imageSize];
	imageSize += fixupSize(luaVMConstantsSize * sizeof(LUA_VM_CONSTANT));
	luaVMStrings = (UINT8 *) &luaVMImage[imageSize];
	imageSize += fixupSize(luaVMStringsSize * sizeof(UINT8));
	luaVMGlobals = (INT32 *) &luaVMImage[imageSize];
	imageSize += fixupSize(luaVMGlobalsSize * sizeof(INT32));

	// Set how much data to read from the file.
	readSize = imageSize;

	// Set the positions of the table pointers for the Lua sections
	// that are not read from the file system. Notice that we fixup 
	// the length of each section to be an even multiple of 4 bytes.
	luaVMRegisters = (INT32 *) &luaVMImage[imageSize];
	imageSize += fixupSize(luaVMRegistersSize * sizeof(UINT8));
	luaVMCallStack = (LUA_VM_CALLSTACK *) &luaVMImage[imageSize];
	imageSize += fixupSize(luaVMCallStackSize * sizeof(UINT8));

	// Prevent overflowing the Lua image buffer.
	if ((imageSize) > LUA_VM_IMAGE_MAX_SIZE) return FALSE;

	// Read the entire Lua program image in a single file system read.
	if (spifs_read(luaFile, (UINT8 *) &luaVMImage, readSize) < readSize) return FALSE;

	// Zero out the registers and call stack.
	memset(luaVMRegisters, 0, fixupSize(luaVMRegistersSize * sizeof(INT32)));
	memset(luaVMCallStack, 0, fixupSize(luaVMCallStackSize * sizeof(UINT8)));

	return TRUE;
}

