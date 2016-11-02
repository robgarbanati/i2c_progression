#include "cmsis_os.h"
#include "Platform.h"
#include "Detector.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvCLK.h"
#include "Driver/DrvTimer.h"
#include "Debug.h"


#define DETECTOR_TIMER_10MS					1875
#define DETECTOR_TIMER_20MS					3750
#define DETECTOR_TIMER_200MS				37500

// Pulse timing for detecting 10 millisecond pulse.  This is
// the timer count assuming a 48MHz clock and 255 prescale.
#define DETECTOR_TIMER_PULSE_MIN		1687
#define DETECTOR_TIMER_PULSE				1875
#define DETECTOR_TIMER_PULSE_MAX		2062

//
// Global Variables
//
BOOL detectorLeft = FALSE;
BOOL detectorRight = FALSE;

//
// Local Functions
//

//
// Global Functions
//

void TMR0_IRQHandler()
{
	// A timeout was detected so indicate no pulse was detected.
	detectorLeft = FALSE;

	// Clear the timer interrupt flag.
	DrvTimer_ClearIntFlagTmr0();
}

void TMR1_IRQHandler()
{
	// A timeout was detected so indicate no pulse was detected.
	detectorRight = FALSE;

	// Clear the timer interrupt flag.
	DrvTimer_ClearIntFlagTmr1();
}

void detectorHandlerLeft(void)
{
	// Determine elapsed time.
	UINT16 count = DrvTimer_GetClkCountTmr0();

	// Was a falling or rising edge detected?
	if (DrvGPIO_GetInputPinValue(&DETECTOR_GPIO, DETECTOR_LEFT_GPIO_PIN) == DETECTOR_LEFT_GPIO_PIN)
	{
		// Rising edge. Is this a valid pulse pulse?
		if ((count > DETECTOR_TIMER_PULSE_MIN) && (count < DETECTOR_TIMER_PULSE_MAX))
		{
			// Indicate a valid pulse has been detected.
			detectorLeft = TRUE;
		}
		else
		{
			// Indicate a valid pulse was not detected.
			detectorLeft = FALSE;
		}
	}
	else
	{
		// Falling edge.  Reset the timer to measure the pulse.
		DrvTimer_ResetCounterTmr0();
		
		// We need to enable the timer after reset.
		DrvTimer_EnableTmr0();
	}
}

void detectorHandlerRight(void)
{
	// Determine elapsed time.
	UINT16 count = DrvTimer_GetClkCountTmr1();

	// Was a falling or rising edge detected?
	if (DrvGPIO_GetInputPinValue(&DETECTOR_GPIO, DETECTOR_RIGHT_GPIO_PIN) == DETECTOR_RIGHT_GPIO_PIN)
	{
		// Rising edge. Is this a valid pulse pulse?
		if ((count > DETECTOR_TIMER_PULSE_MIN) && (count < DETECTOR_TIMER_PULSE_MAX))
		{
			// Indicate a valid pulse has been detected.
			detectorRight = TRUE;
		}
		else
		{
			// Indicate a valid pulse was not detected.
			detectorRight = FALSE;
		}
	}
	else
	{
		// Falling edge.  Reset the timer to measure the pulse.
		DrvTimer_ResetCounterTmr1();
		
		// We need to enable the timer after reset.
		DrvTimer_EnableTmr1();
	}
}

// Initialize the beacon detectors.
void detectorInit(void)
{
	// Set the timer 0 and timer 1 clock source.
	DrvCLK_SetClkSrcTmr0(eDRVCLK_TIMERSRC_48M);
	DrvCLK_SetClkSrcTmr1(eDRVCLK_TIMERSRC_48M);

	// Configure timer 0 and timer 1 with a compare compare value for 200 milliseconds.
	DrvTimer_OpenTmr0(DRVTIMER_PERIODIC_MODE | 255, (UINT16) DETECTOR_TIMER_200MS);
	DrvTimer_OpenTmr1(DRVTIMER_PERIODIC_MODE | 255, (UINT16) DETECTOR_TIMER_200MS, 0);

	// Enable interrupt rising and falling edge.
	DrvGPIO_SetRisingInt(&DETECTOR_GPIO, DETECTOR_LEFT_GPIO_PIN, TRUE);
	DrvGPIO_SetFallingInt(&DETECTOR_GPIO, DETECTOR_LEFT_GPIO_PIN, TRUE);
	DrvGPIO_SetRisingInt(&DETECTOR_GPIO, DETECTOR_RIGHT_GPIO_PIN, TRUE);
	DrvGPIO_SetFallingInt(&DETECTOR_GPIO, DETECTOR_RIGHT_GPIO_PIN, TRUE);
		
	// Enable timer interrupt.
	DrvTimer_EnableIntTmr0();
	DrvTimer_EnableIntTmr1();

	// Enable the timer.
	DrvTimer_EnableTmr0();
	DrvTimer_EnableTmr1();
}

