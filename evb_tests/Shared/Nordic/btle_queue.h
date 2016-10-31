#ifndef __BTLE_QUEUE_H
#define __BTLE_QUEUE_H

#include "global.h"
#include "packet.h"

//
// Global Defines and Declarations
//

//
// Global Functions
//

void btle_init(void);
BOOL btle_put_send_queue(const PACKETData *packet);
BOOL btle_get_send_queue(PACKETData *packet);

#endif // __BTLE_QUEUE_H


