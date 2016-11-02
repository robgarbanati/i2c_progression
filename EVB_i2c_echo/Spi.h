#ifndef __SPI_H
#define __SPI_H

#include "Platform.h"
#include "cmsis_os.h"

//
// Global Defines and Declarations
//

#define SPI_BODY_PACKET_SIGNAL	0x01

extern osThreadId spiThreadID;

#define SPI_HEAD_BUF_LENGTH 31

// Buffer data structure for head.
typedef __packed struct
{
	UINT8 status;
	UINT8 buffer[SPI_HEAD_BUF_LENGTH];
} spiHeadData;

// XXX Note: The Rev B circuit board incorrectly swaps the DO and DI pins 
// XXX so the cable to the head needs have the DO and DI pins swapped.
#define SPI_HEAD_GPIO			GPIOB
#define SPI_HEAD_CS_PIN		DRVGPIO_PIN_1
#define SPI_HEAD_SCK_PIN	DRVGPIO_PIN_2
#define SPI_HEAD_DO_PIN		DRVGPIO_PIN_3
#define SPI_HEAD_DI_PIN		DRVGPIO_PIN_4

//
// Global Functions
//

// Init functions.
void robSpiInit(void);

// Interrupt handler for SPI packets from the body.
void spiHeadHandler(void);

#endif // __SPI_H


