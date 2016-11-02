#include "Global.h"
#include "Idle.h"

// Initialize the idle resources.
void idleInit(void)
{
	// Do nothing for now.
}

// Called by the RTOS idle deamon to perform idle loop processing.  In this 
// function we can do polling and other low priority tasks when no other 
// higher priority tasks are ready to run.
void idleLoop(void)
{
}
