/* 
 * The difference between running as master and as slave is the #define
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
#include "DirDetect.h"

int button1, button2, button3, button4;


void adcClockInit() {
	UINT32	u32HCLK = 0,u32ADCClk = 0;
	
	DrvCLK_SetClkSrcAdc(eDRVCLK_ADCSRC_48M);
	
	DrvCLK_SetClkDividerAdc(48000000/ADC_CLOCK_FREQUENCY);
	
	u32HCLK = DrvCLK_GetHclk();
	u32ADCClk = DrvCLK_GetClkAdc();

	
	//The ADC engine clock must meet the constraint: ADCLK <=  HCKL/2.
	if (u32ADCClk>(u32HCLK/2)) {
		puts("ADCClk is greater than half the frequency of the HCLK. That's bad.\n");
		puts("Check adcClockInit() in Main.c\n");
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
	
	// Enable LDO33.
	DrvCLK_EnableLDO30_P();
	
	// init ADC clock
	adcClockInit();
	
	// Lock protected registers.
	DrvSYS_LockKeyReg();
}

// Initialize system GPIO.
void gpioInit(void) {
	// Configure GPIO A special functions.
	DrvSYS_EnableMultifunctionGpioa(
//		DRVSYS_GPIOA_MF1_SPI0_1ST_CHIP_SEL_OUT |	// Master SPI select output serial flash
//		DRVSYS_GPIOA_MF2_SPI0_CLOCK_OUT |					// Master SPI clock output
//		DRVSYS_GPIOA_MF3_SPI0_DATA_IN |						// Master SPI data input
//		DRVSYS_GPIOA_MF4_SPI0_DATA_OUT |			
		DRVSYS_GPIOA_MF8_ADC_CHANNEL0_IN|  	// ADC input
		DRVSYS_GPIOA_MF9_ADC_CHANNEL1_IN|  	// ADC input
		DRVSYS_GPIOA_MF10_ADC_CHANNEL2_IN|  // ADC input
		DRVSYS_GPIOA_MF11_ADC_CHANNEL3_IN|  // ADC input
		DRVSYS_GPIOA_MF12_ADC_CHANNEL4_IN|  // ADC input
		DRVSYS_GPIOA_MF13_ADC_CHANNEL5_IN  	// ADC input
	);
		
		
	// Configure GPIO port A pins.
	DrvGPIO_SetIOMode(&GPIOA, 
		DRVGPIO_IOMODE_PIN0_QUASI|
		DRVGPIO_IOMODE_PIN1_QUASI|
		DRVGPIO_IOMODE_PIN2_QUASI|
		DRVGPIO_IOMODE_PIN3_QUASI|
		DRVGPIO_IOMODE_PIN4_QUASI|
		DRVGPIO_IOMODE_PIN5_QUASI|
		DRVGPIO_IOMODE_PIN6_QUASI|
		DRVGPIO_IOMODE_PIN7_QUASI
	);
	
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


// Initialize interrupt priorities.
void priorityInit(void) {
	// Set the GPIO interrupt priority high. (low now)
	NVIC_SetPriority(GPAB_IRQn, 0);

	// Set the ADC interrupt lower than SPI and GPIO.
	NVIC_SetPriority(ADC_IRQn, 1);
}


// Main thread.
int main (void) {
	int i, j;
	PRINTD("easy printf\n");
	
	// Initialize clocks.
	clkInit();

	// Initialize GPIO.
	gpioInit();
		
	i2c_init(I2C_SDA_PORT, I2C_SDA_MASK, I2C_SCK_PORT, I2C_SCK_MASK);
	
	// Initialize ADC and DirDetect event handler
//	dirDetectInit();

	
	// Blink to show we're alive.
	while(1);
}

