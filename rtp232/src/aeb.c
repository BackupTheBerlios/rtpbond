//-------------------------------------------------------------------------
// Titel : 
//-------------------------------------------------------------------------
// Funktion : 
// Schaltung : 
//-------------------------------------------------------------------------
// Prozessor : ATmega8
// Takt : 3686400 Hz
// Sprache : 
// Datum : 31.5.2009
// Version : 1.0
// Autor : 
// Programmer: 
// Port : 
//-------------------------------------------------------------------------
// created by myAVR-CodeWizard
//-------------------------------------------------------------------------
//#define 	F_CPU2 	14745600
#include <avr\io.h>
#include <avr\interrupt.h>
#include <string.h>
#include <stdlib.h>


#include "./modules/clock/clock.h"
#include "./modules/timer/timer.h"
#include "./modules/ethernet/ethernet.h"
#include "./modules/serial/serial.h"
#include "./modules/watchdog/watchdog.h"
#include "./hal/com/com1_debug.h"

#include "config.h"




// will be called as soon as all modules are initialized...
// ... it is something like a main method for the eventbased application

extern void onInitApplication();


volatile uint16_t menge2;


volatile uint16_t menge;



volatile uint16_t debug;

//======================================================================
/////////////////////////////////////////////////////////////////////////////
// Main-Funktion
/////////////////////////////////////////////////////////////////////////////
int main()
{
	
  cli();

  debug=0;
  
  #ifdef REQUIRES_BOOTLOADER
    //+***************** BOOTLOADER ONLY ***********************
    // Enable change of interrupt vectors 
    MCUCR = (1<<IVCE);
    // Move interrupts to boot flash section
    MCUCR = (1<<IVSEL);
    //+***************** BOOTLOADER ONLY ***********************
  #else
    // Enable change of interrupt vectors 
    MCUCR = (1<<IVCE);
    // Restore interrupts vectors 
    MCUCR = (0<<IVSEL);
  #endif
  


  // Load configuration from EEPROM if valid, else load fallback config
  onInitConfig();


  // At first, we initialize the real time clock ...
   onInitClock();
  // ... then the timer, it depends on the clock module ...
 onInitTimer();



  // ... Ethernet depends itself on the timer module ...
  onInitEthernet();
  #ifdef REQUIRES_SERIAL
    // ... the same applies to the serial module
    onInitSerial();

com1Initialize(0,0);
  #endif
    
  #ifdef REQUIRES_WATCHDOG
    onInitWatchdog();  
  #endif
  // ... initialization finished, it is save to enable interrupts...
  
  
  onInitApplication();




sei();

	while (1)  	// Mainloop-Begin
	{

	 // Check for timer events
    onTimer();
   
    // COM Event Handler...
   // #ifdef REQUIRES_SERIAL alx
      onSerial();
  //  #endif
           
    onEthernet();

	if(debug>=1)
	 putString_com1("running");


		
	}
	// Mainloop-Ende
}
//---------------------------------------------------------------------------
