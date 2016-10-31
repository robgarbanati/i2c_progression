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

#ifndef LUA_LOADER_H_
#define LUA_LOADER_H_

#include "global.h"
#include "spi_fs.h"

#define LUA_VM_IMAGE_MAX_SIZE		(3 * 1024)

// Memory space for Lua image.
extern UINT8 luaVMImage[];

// Loads the image into memory.
BOOL luaVMImageLoad(SPIFSFile *luaFile);

#endif
