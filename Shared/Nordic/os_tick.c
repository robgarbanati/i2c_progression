#include "global.h"
#include "nrf.h"
#include "nrf_gpio.h"


// Starting the internal LFCLK XTAL oscillator.
static void lfclk_config(void)
{
	NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_LFCLKSTART = 1;
	while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

// Initialize alternative hardware timer as RTX kernel timer
// Return: IRQ number of the alternative hardware timer
int os_tick_init (void)
{
	// Start the internal LFCLK XTAL oscillator.
	lfclk_config();

	// Set prescaler to a TICK of of ~1000Hz.
	NRF_RTC1->PRESCALER = 32;

	// Enable TICK event and TICK interrupt:
	NRF_RTC1->EVTENSET = RTC_EVTENSET_TICK_Msk;
	NRF_RTC1->INTENSET = RTC_INTENSET_TICK_Msk;

	// Start the clock.
	NRF_RTC1->TASKS_START = 1;

	// Return the IRQ number.
	return RTC1_IRQn;
}

// Get alternative hardware timer current value (0 .. OS_TRV)
uint32_t os_tick_val(void)
{
  return 0;
}

// Get alternative hardware timer overflow flag
// Return: 1 - overflow, 0 - no overflow
uint32_t os_tick_ovf(void)
{
  return 0;
}

// Acknowledge alternative hardware timer interrupt
void os_tick_irqack (void)
{
	// Is this a tick condition?
	if ((NRF_RTC1->EVENTS_TICK != 0) && ((NRF_RTC1->INTENSET & RTC_INTENSET_TICK_Msk) != 0))
	{
		// Clear the interrupt?
		NRF_RTC1->EVENTS_TICK = 0;
	}
}
