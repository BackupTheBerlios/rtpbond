#include "./../../hal/rtc/rtc.h"
#include "./clock.h"

uint64_t clockGetTimeStamp()
{
  return rtcGetTime64();
}

uint32_t clockGetShortTimeStamp()
{
  return rtcGetTime32();
}

void onInitClock()
{
  rtcInitialize();
  return;
}

