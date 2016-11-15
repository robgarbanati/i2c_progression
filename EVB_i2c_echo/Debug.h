#ifndef __DEBUG_H
#define __DEBUG_H

#define DEBUG

#ifdef DEBUG

#define PRINTD printf

#else

#define PRINTD(format, args...) ((void)0)

#endif

#endif
