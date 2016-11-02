#include <stdio.h>
#include <string.h>
#include "Global.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvSYS.h"
#include "Driver/DrvCLK.h"
#include "Driver/DrvTimer.h"
#include "Commu/SoftUART.h"
#include "SysClkConfig.h"
#include "DirDetect.h"
#include "Idle.h"
#include "Shell.h"
#include "Spi.h"
#include "SpiFS.h"
#include "XPrintf.h"
#include "DirDetect.h"

// ID of main thread.
//osThreadId mainThreadId;

// Define the various threads.
//osThreadDef(dirDetectThread, osPriorityAboveNormal, 1, 400);

// Handle the SPI1 interrupt.
void SPI1_IRQHandler()
{
	SoftUART_SpiIsr();
}
	   
// Handle the GPIO interrupt.
void GPAB_IRQHandler()
{
	SoftUART_GpioIsr();
}


void adcClockInit() {
	UINT32	u32HCLK = 0,u32ADCClk = 0;
	
	DrvCLK_SetClkSrcAdc(eDRVCLK_ADCSRC_48M);
	
	DrvCLK_SetClkDividerAdc(48000000/ADC_CLOCK_FREQUENCY);
	
	u32HCLK = DrvCLK_GetHclk();
	u32ADCClk = DrvCLK_GetClkAdc();

	
	//The ADC engine clock must meet the constraint: ADCLK <=  HCKL/2.
	if (u32ADCClk>(u32HCLK/2)) {
		xputs("ADCClk is greater than half the frequency of the HCLK. That's bad.\n");
		xputs("Check adcClockInit() in Main.c\n");
	}
}

// Initialize system clock.
void clkInit(void)
{
	// Unlock protected registers.
	DrvSYS_UnlockKeyReg();

	// Configure the system clock source.  See SysClkConfig.h.
	_SYSCLKCONFIG();

	// Enable LDO33.
	DrvCLK_EnableLDO30_P();
	
	// init ADC clock
	adcClockInit();

	// Lock protected registers.
	DrvSYS_LockKeyReg();
}

// Initialize interrupt priorities.
void priorityInit(void)
{
	// Set the SPI slave interrupt priority high.
	NVIC_SetPriority(SPI1_IRQn, 0);

	// Set the GPIO interrupt priority high.
	NVIC_SetPriority(GPAB_IRQn, 0);

	// Set the ADC interrupt lower than SPI and GPIO.
	NVIC_SetPriority(ADC_IRQn, 1);
}

// Initialize system GPIO.
void gpioInit(void)
{
	uint32_t i = 0;
	// Configure GPIO A special functions.
	DrvSYS_EnableMultifunctionGpioa(
//		DRVSYS_GPIOA_MF1_SPI0_1ST_CHIP_SEL_OUT |	// Master SPI select output serial flash
//		DRVSYS_GPIOA_MF2_SPI0_CLOCK_OUT |					// Master SPI clock output
//		DRVSYS_GPIOA_MF3_SPI0_DATA_IN |						// Master SPI data input
//		DRVSYS_GPIOA_MF4_SPI0_DATA_OUT |					// Master SPI data output
		
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

	// Configure GPIO port B pins.
	DrvGPIO_SetIOMode(&GPIOB,
		DRVGPIO_IOMODE_PIN6_OUT|
		DRVGPIO_IOMODE_PIN7_OUT|
		DRVGPIO_IOMODE_PIN12_QUASI|
		DRVGPIO_IOMODE_PIN13_QUASI|
		DRVGPIO_IOMODE_PIN14_QUASI|
		DRVGPIO_IOMODE_PIN15_QUASI
	);
//	DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_6);
//	DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_7);

//	for(;;) {
////		nonstatic_i2c_send_byte(&i2c_puck, 0xAA);
//		DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_6);
//		DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_7);
//		for(i=0;i<1000000;i++);
//		DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_6);
//		DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_7);
//		for(i=0;i<1000000;i++);
//	}
	// Release the slave select line (inactive high) to the SPI slaves.
	// Even if we aren't using a peripheral we should release the slave
	// select line to make sure the device doesn't put data on the bus.
	//DrvGPIO_SetOutputBit(&SPI_SFLASH_GPIO, SPI_SFLASH_SS_PIN);
}

// Initializet the soft UART.
void uartInit(void)
{
	S_SOFTUART_INTF uartIntf;
	S_SOFTUART_UART_CONF uartConf;	

	// Configure the soft uart configuration.
	uartConf.u32BaudRate = SOFTUART_BAUD_9600;
	uartConf.u8DataBits = SOFTUART_DATABITS_8;
	uartConf.u8Parity = SOFTUART_PARITY_NONE;
	uartConf.u8StopBits = SOFTUART_STOPBITS_1;

	// The soft uart only supports SPI1 for N572F072.
	uartIntf.psTimer = (TMR_T *) TMR0_BASE;
	uartIntf.sSPIIntf.psSPIIntf = (SPI_T *) SPI1_BASE;

	// Open the soft uart driver.
	SoftUART_Open(&uartConf, &uartIntf, 0);
}

// Interface formatted output to soft UART.
static void xprintfOutput(char ch)
{
	SoftUART_PutChar((CHAR) ch);
}

// Interface formatted input to soft UART.
static unsigned char xprintfInput(void)
{
	UINT8 ch;
	while (!SoftUART_GetChar((PCHAR) &ch));
	return ch;
}

// Initialize formatted IO.
void xprintfInit(void)
{
	// Connected formatted input/output to soft UART.
	xdev_in(xprintfInput);
	xdev_out(xprintfOutput);
}

// Initialize the other threads we use.
//void threadInit(void)
//{
//	// Create the service threads.
//	dirDetectThreadId = osThreadCreate(osThread(dirDetectThread),  NULL);
//}

// Main thread.
int main (void)
{
	// Set the main thread id.
//	mainThreadId = osThreadGetId();
	
	// Initialize clocks.
	clkInit();

	// Initialize interrupt priorities.
	priorityInit();

	// Initialize GPIO.
	gpioInit();
	
	// Initialize the SPI.
	//spiInit();
	
//	// Initialize the soft uart.
//	uartInit();
//	
//	// Initialize formatted IO.
//	xprintfInit();	// Working loop.

	// Initialize ADC and DirDetect event handler
	dirDetectInit();

//	// Initialize idle processing.
//	idleInit();

//	// Initialize other threads.
//	threadInit();

//	// Mount the file system.
//	spiFSMount();

//	// Call the shell.
//	shell();
//	
//	// Unmount the file system.
//	spiFSUnmount();

	// Spin here.
	for (;;);
}

