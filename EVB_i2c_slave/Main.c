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
#include "soft_i2c.h"

SoftI2C i2c_inst;

int button1, button2, button3, button4;

// Handle the GPIO interrupt. Print a message and clear the int flags.
void GPAB_IRQHandler(void) {
	static int count = 0;
//	int success;
	uint16_t int_flags = DrvGPIO_GetIntFlag(I2C_SCK_PORT, 0xFFFF);
	
	// Did interrupt fire from a clock transition?
	if(int_flags & I2C_SCK_MASK) {
		DrvGPIO_ClearIntFlag(I2C_SCK_PORT, I2C_SCK_MASK);
		
		// Did clock rise from low to high?
		if(i2c_clock_read()) {
			i2c_update_slave_state(SCK_ROSE);
		} else {
			i2c_update_slave_state(SCK_FELL);
		}
	}
	
	// Did interrupt fire from a data transition?
	if(int_flags & I2C_SDA_MASK) {
		DrvGPIO_ClearIntFlag(I2C_SDA_PORT, I2C_SDA_MASK);
		
		// Did clock rise from low to high?
		if(i2c_data_read()) {
			i2c_update_slave_state(SDA_ROSE);
		} else {
			i2c_update_slave_state(SDA_FELL);
		}
	}
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
	
	// Enable interrupt on any transition for I2C SDA and SCK
	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_14, TRUE);
	DrvGPIO_SetRisingInt(&GPIOB, DRVGPIO_PIN_14, TRUE);
	DrvGPIO_SetFallingInt(&GPIOB, DRVGPIO_PIN_15, TRUE);
	DrvGPIO_SetRisingInt(&GPIOB, DRVGPIO_PIN_15, TRUE);
	

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
	
	i2c_init(I2C_SDA_PORT, I2C_SDA_MASK, I2C_SCK_PORT, I2C_SCK_MASK);

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

