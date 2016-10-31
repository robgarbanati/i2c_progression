#ifndef __SPI_H
#define __SPI_H

#include "Platform.h"
//#include "cmsis_os.h"

//
// Global Defines and Declarations
//

#define SPI_BODY_PACKET_SIGNAL	0x01


#define SPI_BUF_LENGTH 8

// Buffer data structure for head.
typedef __packed struct
{
	UINT8 status;
	UINT8 buffer[SPI_BUF_LENGTH];
} spiHeadData;

// XXX Note: The Rev B circuit board incorrectly swaps the DO and DI pins 
// XXX so the cable to the head needs have the DO and DI pins swapped.
#define SPI_GPIO			GPIOB
#define SPI_CS_PIN		DRVGPIO_PIN_1
#define SPI_SCK_PIN		DRVGPIO_PIN_2
#define SPI_DO_PIN		DRVGPIO_PIN_3
#define SPI_DI_PIN		DRVGPIO_PIN_4

//
// Global Functions
//


//
// open spi slave driver to talk to Sway Host
//
void spiSlave_Init(void);
//
// close spi slave driver
//
void spiSlave_Close(void);
// open spi driver
void spiMaster_Init(void);
//
// close spi driver
//
void spiMaster_close(void);
//
// return 8 bits read from spi.
//
static UINT8 spiMaster_Read8(void);
//
// write a value of 8 bits to an address
//
void spiMaster_Write8(UINT8 value);
//
// Init functions.
//

// Interrupt handler for SPI packets from the body.
void read_and_write_SPI(void);

#endif // __SPI_H


