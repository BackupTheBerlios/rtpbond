#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>

uint64_t clockGetTimeStamp();
void onInitClock();
uint32_t clockGetShortTimeStamp();

#endif /*CLOCK_H_*/
