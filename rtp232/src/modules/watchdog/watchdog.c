#ifdef REQUIRES_WATCHDOG

  #include "./watchdog.h"
  #include "./../timer/timer.h"
    
  // http://www.nongnu.org/avr-libc/user-manual/group__avr__watchdog.html

  void onWatchdogTimeout()
  {
    wdt_reset();
    timerAddTimeout(WATCHDOG_RESET_TIMEOUT,onWatchdogTimeout);
  }
  
  void watchdogDisable()
  {
    wdt_reset();
    wdt_disable();    
  }
  
  void onInitWatchdog()
  {
    wdt_reset();
    wdt_enable(WATCHDOG_TIMEOUT);
    
    timerAddTimeout(WATCHDOG_RESET_TIMEOUT,onWatchdogTimeout);
  }
#endif
