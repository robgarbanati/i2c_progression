/**
 *
 * @file Packet.h
 *
 * This header file defines Packet
 * handling functions.
 *
 * Author: brandon@ologicinc.com
 * Create date: 1/11/2016
 */

#ifndef __PACKET_H__
#define __PACKET_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvSYS.h"
#include "Driver/DrvCLK.h"
#include "Driver/DrvSPI.h"
#include "Debug.h"

/************************** Constants Definitions *********************/

// Maximum amount of data to send in a packet.
#define PACKET_BUF_LENGTH 		20

// Number of packets that can be stored in a PACKETQueue.
#define PACKET_QUEUE_DEPTH		5

/************************** Type Prototypes **************************/

// Packet buffer data structure.
typedef __packed struct
{
    uint8_t length;
    uint8_t checksum;
    uint8_t buffer[PACKET_BUF_LENGTH];
} PACKETData;

typedef struct _PACKETQueue
{
    uint8_t head;
    uint8_t tail;
    PACKETData packet[PACKET_QUEUE_DEPTH];
} PACKETQueue;

/******************* Global Inline Functions *************************/

static __inline uint8_t
packetGetType(const PACKETData * packet)
{
    uint8_t *data = (uint8_t *) packet;
    return data[2];
}

static __inline uint8_t
packetGetDest(const PACKETData * packet)
{
    uint8_t *data = (uint8_t *) packet;
    return data[3];
}

static __inline void
packetSetDest(PACKETData * packet, uint8_t dest)
{
    uint8_t *data = (uint8_t *) packet;
    data[3] = dest;
}

/*
static __inline uint8_t packetGetSource(const PACKETData *packet)
{
	return ((*(packet->buffer) >> 4) & 0x03);
}

static __inline void packetSetSource(PACKETData *packet, uint8_t source)
{
	*(packet->buffer) = (*(packet->buffer) & 0xcf) | ((source << 4) & 0x30);
}

static __inline void packetSetType(PACKETData *packet, uint8_t type)
{
	*(packet->buffer) = (*(packet->buffer) & 0xf0) | (type & 0x0f);
}
*/

static __inline void
packetSetChecksum(PACKETData * packet)
{
    uint8_t i;
    packet->checksum = packet->length;
    for (i = 0; i < packet->length; ++i)
    {
        packet->checksum += packet->buffer[i];
    }
}

static __inline bool
packetValidateChecksum(PACKETData * packet)
{
    uint8_t i, checksum = packet->length;
    for (i = 0; i < packet->length; ++i)
        checksum += packet->buffer[i];
    // XXX PRINTD("packet->checksum: %d\n",packet->checksum);
    // XXX PRINTD("checksum: %d\n",checksum);
    return packet->checksum == checksum;
}

static __inline void
packetInit(PACKETData * packet, uint8_t length, uint8_t * buffer)
{
    uint8_t i;
    packet->length = length;
    packet->checksum = packet->length;
    for (i = 0; i < packet->length; ++i)
    {
        packet->buffer[i] = buffer[i];
        packet->checksum += packet->buffer[i];
    }

}

static __inline void
packetPrint(PACKETData * packet)
{
    uint8_t i;
    uint8_t *data = (uint8_t *) packet;
    for (i = 0; i < packet->length + 2; ++i)
        PRINTD("data[%d]=%d\n", i, data[i]);
    if (packetValidateChecksum(packet))
        PRINTD("checksum PASSED.\n");
    else
        PRINTD("checksum FAILED.\n");

    PRINTD("\n");
}

/************************** Function Prototypes **********************/

/*
void get_upstream_packet(void);
void get_downstream_packet(void);

void init_outgoing_downstream_packet(void);
void send_downstream_packet(void);
*/

/************************ Variable Declarations **********************/

/*
extern PACKETQueue iuq;		// Incoming Upstream Queue
extern PACKETQueue idq; 	// Incoming Downstream Queue
extern PACKETQueue ouq;		// Outgoing Upstream Queue
extern PACKETQueue odq; 	// Outgoint Downstream Queue
*/

#endif // __PACKET_H__
