#ifndef __OS_TICK_H
#define __OS_TICK_H

#include "global.h"

int os_tick_init (void);
uint32_t os_tick_val(void);
uint32_t os_tick_ovf(void);
void os_tick_irqack (void);

#endif // __OS_TICK_H
