#include <string.h>
#include "Global.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvSPI.h"
#include "Driver/DrvSYS.h"
#include "Packet.h"
#include "Spi.h"
#include "Debug.h"

//
// Global Variables
//

// SPI master bus mutex.
//osMutexId spiMasterMutex;
//osMutexDef(spiMasterMutex);

//
// Global Functions
//

void spiInit(void)
{
	// Create the SPI mutex.
//	spiMasterMutex = osMutexCreate(osMutex(spiMasterMutex));
}

