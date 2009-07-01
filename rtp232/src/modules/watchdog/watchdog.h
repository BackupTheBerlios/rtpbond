#ifdef REQUIRES_WATCHDOG
  #ifndef WATCHDOG_H_
    #define WATCHDOG_H_
    
    #include <avr/wdt.h>
 
    // the watchdog timeout
    #ifndef WATCHDOG_TIMEOUT
      #define WATCHDOG_TIMEOUT WDTO_1S
    #endif

    // the timeout intended intervall between watchdog resets
    #ifndef WATCHDOG_RESET_TIMEOUT
      #define WATCHDOG_RESET_TIMEOUT 512
    #endif

    void watchdogDisable();
    void onInitWatchdog();
  
  #endif /*WATCHDOG_H_*/
#endif
