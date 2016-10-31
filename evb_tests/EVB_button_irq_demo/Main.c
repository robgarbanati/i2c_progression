/* 
 * This demo is meant to be used with the debugger and the N572 eval board.
 * It simply demonstrates use of printf and the GPAB interrupt.
 * If you press 4 of the 6 buttons, GPAB_IRQHandler will fire, 
 * printing "in irq".
 * Sometimes the interrupt will fire twice. I'm not sure why that is.
 * It might have to do with poor button debouncing on the Nuvoton eval board.
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

int button1, button2, button3, button4;

// Handle the GPIO interrupt. Print a message and clear the int flags.
void GPAB_IRQHandler(void) {
	printf("in irq\n");
	
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
		DRVGPIO_IOMODE_PIN12_QUASI	|									// Button 1 input
		DRVGPIO_IOMODE_PIN13_QUASI	|									// Button 2 input
		DRVGPIO_IOMODE_PIN14_QUASI	|									// Button 3 input
		DRVGPIO_IOMODE_PIN15_QUASI										// Button 4 input
	);
	
	// Enable interupt on falling button press
	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_12, TRUE);
	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_13, TRUE);
	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_14, TRUE);
	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_15, TRUE);
	

	// Enable GPIO interrupt routine.
	NVIC_EnableIRQ(GPAB_IRQn);    
}


// Main thread.
int main (void) {
	printf("easy printf\n");
	
	// Initialize clocks.
	clkInit();

	// Initialize GPIO.
	gpioInit();

	for (;;) {
		button1 = DrvGPIO_GetInputPinValue(&GPIOB, DRVGPIO_PIN_12);
		button2 = DrvGPIO_GetInputPinValue(&GPIOB, DRVGPIO_PIN_13);
		button3 = DrvGPIO_GetInputPinValue(&GPIOB, DRVGPIO_PIN_14);
		button4 = DrvGPIO_GetInputPinValue(&GPIOB, DRVGPIO_PIN_15);
	}
}

