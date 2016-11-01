/* 
 * The difference between running as master and as slave is the #define // TODO where is the #define?
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

int button1, button2, button3, button4;

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
		DRVGPIO_IOMODE_PIN14_QUASI	|			// I2C Clock
		DRVGPIO_IOMODE_PIN15_QUASI	|			// I2C Data
		DRVGPIO_IOMODE_PIN13_QUASI		|		// Button 3 input
		DRVGPIO_IOMODE_PIN12_QUASI				// Button 4 input
	);
	
	// Enable interrupt on any transition for I2C SDA and SCK
	DrvGPIO_SetFallingInt(I2C_SCK_PORT, I2C_SCK_MASK, TRUE);
	DrvGPIO_SetRisingInt(I2C_SCK_PORT, I2C_SCK_MASK, TRUE);
	DrvGPIO_SetFallingInt(I2C_SDA_PORT, I2C_SDA_MASK, TRUE);
	DrvGPIO_SetRisingInt(I2C_SDA_PORT, I2C_SDA_MASK, TRUE);
	

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

