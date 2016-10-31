/*****************************************************************************
*
* @file luaLib.c
*
* This file implements C functions that can be
* called from a Lua script. 
*
* Copyright (c) 2007, 2008 Brandon Blodget
* Copyright (c) 2013 OLogic Inc.
* All rights reserved.
*
* Author: Brandon Blodget
*
*****************************************************************************/

#include "global.h"
#include "lua_vm.h"
#include "lua_lib.h"
#include "xprintf.h"

#if 0
static UINT8 clock_A;
static UINT8 clock_B;
static UINT32 debounced_state;

static void debounce(void)
{
  unsigned delta;
  unsigned changes;
	UINT32 new_sample;
	
	DrvTimer_WaitMillisecondTmr2(1);
	
	new_sample = DrvGPIO_GetInputPinValue(&GPIOA, DRVGPIO_PIN_6 | DRVGPIO_PIN_7);
	new_sample |= DrvGPIO_GetInputPinValue(&GPIOB, DRVGPIO_PIN_12 | DRVGPIO_PIN_13 | 
			DRVGPIO_PIN_14 | DRVGPIO_PIN_15);

  delta = new_sample ^ debounced_state;   //Find all of the changes

  clock_A ^= clock_B;                     //Increment the counters
  clock_B  = ~clock_B;

  clock_A &= delta;                       //Reset the counters if no changes
  clock_B &= delta;                       //were detected.

  changes = ~(~delta | clock_A | clock_B);
  debounced_state ^= changes;
}
#endif

// Delays the indicated number of milliseconds.
void luaSysDelay(UINT32 delay)
{
#if 0
	DrvTimer_WaitMillisecondTmr2((UINT32) delay);
#endif
}

// Gets the state of the pushbuttons.  Returns value composed 
// of bits representing button state ... [b5,b4,b3,b2,b1,b0]
// If no pushbuttons have been pressed since the last check it 
// returns 0. Clears buttons state before exiting.
UINT32 luaSysGetButton(void)
{
#if 0
	CHAR i;
	for (i=0; i<20; i++)
	{
		debounce();
	}
	return debounced_state;
#endif
	return 0;
}

// Sets the state of the leds on the board.
// led_num:  The target led (1-6)
// state:    1=ON, else OFF
void luaSysSetLED(UINT8 ledNum, UINT8 state)
{
#if 0
	switch(ledNum)
	{
		case 1 : 
			if (state == 1)
			{
				DrvGPIO_ClearOutputBit(&GPIOB,DRVGPIO_PIN_6);
			}
			else
			{
				DrvGPIO_SetOutputBit(&GPIOB,DRVGPIO_PIN_6);
			}
			break;
		case 2 : 
			if (state == 1)
			{
				DrvGPIO_ClearOutputBit(&GPIOB,DRVGPIO_PIN_8);
			}
			else
			{
				DrvGPIO_SetOutputBit(&GPIOB,DRVGPIO_PIN_8);
			}
			break;
		case 3 : 
			if (state == 1)
			{
				DrvGPIO_ClearOutputBit(&GPIOB,DRVGPIO_PIN_10);
			}
			else
			{
				DrvGPIO_SetOutputBit(&GPIOB,DRVGPIO_PIN_10);
			}
			break;
		case 4 : 
			if (state == 1)
			{
				DrvGPIO_ClearOutputBit(&GPIOB,DRVGPIO_PIN_11);
			}
			else
			{
				DrvGPIO_SetOutputBit(&GPIOB,DRVGPIO_PIN_11);
			}
			break;
		case 5 : 
			if (state == 1)
			{
				DrvGPIO_ClearOutputBit(&GPIOB,DRVGPIO_PIN_9);
			}
			else
			{
				DrvGPIO_SetOutputBit(&GPIOB,DRVGPIO_PIN_9);
			}
			break;
		case 6 : 
			if (state == 1)
			{
				DrvGPIO_ClearOutputBit(&GPIOB,DRVGPIO_PIN_7);
			}
			else
			{
				DrvGPIO_SetOutputBit(&GPIOB,DRVGPIO_PIN_7);
			}
			break;
	}
#endif
}

