#include "global.h"
#include "app_error.h"
#include "nrf_soc.h"

static BOOL os_idle_process_events = FALSE;

// Called to enable event processing in the idle loop.
void os_idle_enable(BOOL enable)
{
	// Enable/disable event processing.
	os_idle_process_events = enable;
}

// The idle demon is a system thread, running when no other thread is ready to run.
void os_idle_demon(void)
{
  for (;;)
	{
		// Should we process SoftDevice events?
		if (os_idle_process_events)
		{
			// Waits for an application event.
			uint32_t err_code = sd_app_evt_wait();
			APP_ERROR_CHECK(err_code);
		}
  }
}

