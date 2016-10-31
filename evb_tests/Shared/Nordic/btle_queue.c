#include <string.h>
#include "global.h"
#include "ble_spi.h"
#include "btle_queue.h"

//
// Send/Receive Queues and Flags
//

// BTLE queue mutex.
osMutexId btleQueueMutex;
osMutexDef(btleQueueMutex);

// Must be a power of two.
#define BTLE_SEND_QUEUE_SIZE 8
#define BTLE_SEND_QUEUE_MASK (BTLE_SEND_QUEUE_SIZE - 1)

// Send btle queue state variables.
static UINT8 btleSendQueueTail = 0;
static UINT8 btleSendQueueHead = 0;
static PACKETData btleSendQueue[BTLE_SEND_QUEUE_SIZE];

//
// Global Functions
//

void btle_init(void)
{
	// Create the queue mutex.
	btleQueueMutex = osMutexCreate(osMutex(btleQueueMutex));
}

// Put a packet on the send queue.
BOOL btle_put_send_queue(const PACKETData *btle)
{
	BOOL status = FALSE;

	// Sanity check the packet length.
	if ((btle->length >= 1) && (btle->length <= PACKET_BUF_LENGTH))
	{
		// Wait for exclusive access to the queue.
		osMutexWait(btleQueueMutex, osWaitForever);

		// Increment the receive queue head.
		btleSendQueueHead = (btleSendQueueHead + 1) & BTLE_SEND_QUEUE_MASK;

		// Prevent collision of the receive queue head by discarding old packets.
		if (btleSendQueueHead == btleSendQueueTail) 
			btleSendQueueTail = (btleSendQueueTail + 1) & BTLE_SEND_QUEUE_MASK;

		// Copy the packet into the send queue.
		memcpy(&btleSendQueue[btleSendQueueHead], btle, sizeof(PACKETData));

		// Release exclusive access to the queue.
		osMutexRelease(btleQueueMutex);

		// We were able to queue the data.
		status = TRUE;
	}

	// Signal the ble thread that a packet is waiting to be sent.
	osSignalSet(bleThreadId, 0x01);

	return status;
}

// Get the next packet from the send queue.
BOOL btle_get_send_queue(PACKETData *btle)
{
	BOOL status = FALSE;

	// Wait for exclusive access to the queue.
	osMutexWait(btleQueueMutex, osWaitForever);

	// Is there a packet on the queue?
	if (btleSendQueueTail != btleSendQueueHead)
	{
		// Increment the receive queue tail.
		btleSendQueueTail = (btleSendQueueTail + 1) & BTLE_SEND_QUEUE_MASK;

		// Copy packet data from the receive queue.
		memcpy(btle, &btleSendQueue[btleSendQueueTail], sizeof(PACKETData));

		// We were able to get a packet from the queue.
		status = TRUE;
	}

	// Release exclusive access to the queue.
	osMutexRelease(btleQueueMutex);

	return status;
}

