/* 
 * This demo is meant to be used with the debugger and the N572 eval board.
 * It simply demonstrates use of printf and the GPAB interrupt.
 * If you press 4 of the 6 buttons, GPAB_IRQHandler will fire, 
 * printing "in irq".
 * Sometimes the interrupt will fire twice. I'm not sure why that is.
 * It might have to do with faulty edge detection on the Nuvoton.
 * To view the printfs, start the debugger, click run, and click
 * View->Serial Windows->UART #1.
 */ 

#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvSYS.h"
#include "Driver/DrvCLK.h"
#include "Driver/DrvSPI.h"
#include "SysClkConfig.h"
#include "Debug.h"
#include "gpio_rw.h"

int button1, button2, button3, button4;

// Handle the GPIO interrupt. Print a message and clear the int flags.
void GPAB_IRQHandler(void) {
	static int count = 0;
	printf("in irq\n");
	
	// Is clock also high (which means this is a stop condition)?
	if(DrvGPIO_GetInputPinValue(I2C_SCK_PORT, I2C_SCK_MASK) != 0) {
		// TODO do something useful
		printf("got stop condition number %d.\n", count++);
//		i2c_respond_to_master();
	}
	
	// Clear the SPI CS interrupt.
	DrvGPIO_ClearIntFlag(&GPIOB, GPIO_PIN_PIN12);
	DrvGPIO_ClearIntFlag(&GPIOB, GPIO_PIN_PIN13);
	DrvGPIO_ClearIntFlag(&GPIOB, GPIO_PIN_PIN14);
	DrvGPIO_ClearIntFlag(&GPIOB, GPIO_PIN_PIN15);
}

// Initialize system clock.
void clkInit(void)
{
	// Unlock protected registers.
	DrvSYS_UnlockKeyReg();

	// Configure the system clock source.  See SysClkConfig.h.
	_SYSCLKCONFIG();
	
	// Enable power on reset (assert reset when power first comes on).
	DrvSYS_ClearPORDisableCode_P();

	// Lock protected registers.
	DrvSYS_LockKeyReg();
}

// Initialize system GPIO.
void gpioInit(void) {
	
	DrvGPIO_EnableInputPin(&GPIOB,
		DRVGPIO_PIN_12 |
		DRVGPIO_PIN_13 |
		DRVGPIO_PIN_14 |
		DRVGPIO_PIN_15
	);

	// Configure GPIO port B pins.
	DrvGPIO_SetIOMode(&GPIOB,
		DRVGPIO_IOMODE_PIN6_OPEN_DRAIN	|		// LED 1
		DRVGPIO_IOMODE_PIN7_OPEN_DRAIN	|		// LED 2
		DRVGPIO_IOMODE_PIN14_QUASI	|		// I2C Clock
		DRVGPIO_IOMODE_PIN15_QUASI	|		// I2C Data
		DRVGPIO_IOMODE_PIN13_QUASI		|		// Button 3 input
		DRVGPIO_IOMODE_PIN12_QUASI				// Button 4 input
	);
	
	// Enable interupt on falling button press
//	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_12, TRUE);
//	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_13, TRUE);
	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_14, TRUE);
//	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_15, TRUE);
	

	// Enable GPIO interrupt routine.
	NVIC_EnableIRQ(GPAB_IRQn);    
}


// Main thread.
int main (void) {
	int i;
	PRINTD("easy printf\n");
	
	// Initialize clocks.
	clkInit();

	// Initialize GPIO.
	gpioInit();

	// Blink to show we're alive.
	for(;;) {
		DrvGPIO_SetOutputBit(&GPIOB, 1 << 6);
		DrvGPIO_ClearOutputBit(&GPIOB, 1 << 7);
		for(i=0;i<1000000;i++);
		DrvGPIO_ClearOutputBit(&GPIOB, 1 << 6);
		DrvGPIO_SetOutputBit(&GPIOB, 1 << 7);
		for(i=0;i<1000000;i++);
	}
}

