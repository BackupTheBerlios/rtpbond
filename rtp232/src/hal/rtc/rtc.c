#include "./rtc.h"
#include "../com/com0.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>

/*
 * A watch crystal (32.768kHz) causes a 8-Bit counter 
 * to overflow exactly 128 times per second, because 
 * 256*126 = 32768. If you devide 1 second by 32768
 * you get an resolution of 30µs. 
 * 
 *      Prescaler      |  Prescaled Counter  | Overflows    | Maximal  
 *  CS02 | CS01 | CS00 |       Clock         |   per Second |    Resolution
 * ------+------+------+---------------------+--------------+---------------- 
 *    0  |   0  |   1  |  32798/1 (32768 Hz) |     128,000  |    30,51 µs 
 *    0  |   1  |   0  |  32768/8  (4096 Hz) |      16,000  |   244,14 µs
 *    0  |   1  |   1  |  32768/32 (1024 Hz) |       4,000  |   976,56 µs   (**)
 *    1  |   0  |   0  |  32768/64  (512 Hz) |       2,000  |     1,95 ms
 *    1  |   0  |   1  |  32768/128 (256 Hz) |       1,000  |     3,90 ms
 *    1  |   1  |   0  |  32768/256 (128 Hz) |       0,500  |     7,81 ms
 *    1  |   1  |   1  |  32768/1024 (32 Hz) |       0,125  |    31,25 ms 
 *   
 * Within internet protocols only ms and above are used, therefor a prescale 
 * devider of 32, should offer the requested resolution.
 *  
 */



// the overflow counter will warp approx every 34 years...
// ... which is a lot more than the expected livetime of the device
volatile uint32_t overflows;




ISR(TIMER0_OVF_vect)
{
  // we should get 4 interrupts per Second... 
  TCNT0 = 0;
  overflows++;
}



void rtcInitialize()
{



  overflows = 0;
  
  // We use an external Timer clock, this means...
  // ...the timer is asynchronous to out system clock...
  // ... according to the ATMega manual, the following...
  // ... precausions are mandatory.
  
  // Disable interrputs OCIE0 and TOIE0...
  TIMSK &= ~( (1<<OCIE0) | (1<<TOIE0) );
  // ... switch to asynchronous mode...
  ASSR |= (1 << AS0);
  // ... clear TCNT0, OCR0 ...
  TCNT0 = 0;
  OCR0 = 0;
  // ... clear and set TCCR0 to the 32 prescal divider
  TCCR0 = (1 << CS00) |  (1<<CS01);
  
  // now wait for TCN0UB
  while (!(ASSR & (1<< TCN0UB)));
  
  // Clear Timer Infterrput flags...
  TIFR &= ~( (1<<OCF0) | (1<<TOV0));  
  
  // Enable Overflow Interrupt 
  TIMSK |= (1 << TOIE0); 


  
}

uint64_t rtcGetTime64()
{
  return (overflows << 8) + TCNT0;
}

uint32_t rtcGetTime32()
{
  // will cut of the upper 8 bits...
  return (overflows << 8) + TCNT0;
}

uint16_t rtcGetTime16()
{
  // will cut of the upper 8 bits...
  return (overflows << 8) + TCNT0;
}

