#ifndef RTC_H_
#define RTC_H_

#include <stdint.h>

void rtcInitialize();
uint64_t rtcGetTime64();
uint32_t rtcGetTime32();
uint16_t rtcGetTime16();

#endif /*SYSTEMTIMER_H_*/
