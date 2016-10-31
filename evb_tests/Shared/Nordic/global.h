#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <stdint.h>
#include <stdbool.h>
#include "platform.h"
#include "cmsis_os.h"
#include "nordic_common.h"

//
// Global Defines
//
#define UNUSED(p) if(p){}

//
// Global Types
//

//
// Global Functions
//
extern osThreadId mainThreadId;

// Report conflicts with functions using heap storage.
#pragma import(__use_no_heap)
	
#endif // __GLOBAL_H


