#ifndef __PACKET_H
#define __PACKET_H

#include "global.h"

//
// Global Defines and Declarations
//

// Packet routing locations (source/destination).
#define PACKET_LOC_BTLE 			0
#define PACKET_LOC_MCU0				1
#define PACKET_LOC_MCU1				2
#define PACKET_LOC_MCU2				3

// Packet types.
#define PACKET_TYPE_SERIAL		0

// Maximum amount of data to send in a packet.
#define PACKET_BUF_LENGTH 20

// Packet buffer data structure.
typedef __packed struct
{
	UINT8 length;
	UINT8 checksum;
	UINT8 buffer[PACKET_BUF_LENGTH];
} PACKETData;

// 1 Byte Packet Header Format
//
// Bits    [7][6][5][4][3][2][1][0]
//         |    ||    ||          |
//         |    ||    ||          |
//         |    ||    ||           \__
//         |    ||    | \_____________ Type
//         |    ||    |
//         |    ||     \______________
//         |    | \___________________ Source
//         |    |
//         |     \____________________
//          \_________________________ Destination

//
// Global Functions
//

static __inline UINT8 packet_get_dest(const PACKETData *packet)
{
	return ((*(packet->buffer) >> 6) & 0x03);
}

static __inline UINT8 packet_get_source(const PACKETData *packet)
{
	return ((*(packet->buffer) >> 4) & 0x03);
}

static __inline UINT8 packet_get_type(const PACKETData *packet)
{
	return (*(packet->buffer) & 0x0f);
}

static __inline void packet_set_dest(PACKETData *packet, UINT8 dest)
{
	*(packet->buffer) = (*(packet->buffer) & 0x3f) | ((dest << 6) & 0xc0);
}

static __inline void packet_set_source(PACKETData *packet, UINT8 source)
{
	*(packet->buffer) = (*(packet->buffer) & 0xcf) | ((source << 4) & 0x30);
}

static __inline void packet_set_type(PACKETData *packet, UINT8 type)
{
	*(packet->buffer) = (*(packet->buffer) & 0xf0) | (type & 0x0f);
}

static __inline void packet_set_checksum(PACKETData *packet)
{
	UINT8 i;
	packet->checksum = packet->length;
	for (i = 0; i < packet->length; ++i) packet->checksum += packet->buffer[i];
}

static __inline BOOL packet_validate_checksum(PACKETData *packet)
{
	UINT8 i, checksum = packet->length;
	for (i = 0; i < packet->length; ++i) checksum += packet->buffer[i];
	return packet->checksum == checksum;
}

static __inline void packet_init(PACKETData *packet, UINT8 length, UINT8 *buffer)
{
	UINT8 i;
	packet->length = length;
	packet->checksum = packet->length;
	for (i = 0; i < packet->length; ++i) packet->checksum += (packet->buffer[i] = buffer[i]);
}

#endif // __PACKET_H


