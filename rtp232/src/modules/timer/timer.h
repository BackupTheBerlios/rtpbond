#ifndef TIMER_H_
#define TIMER_H_

#ifndef REQUIRES_DOUBLYLINKEDLIST
  #error "timer.h requires DOUBLYLINKEDLIST"
#endif

#include <stdint.h>
#include "./../../toolkit/doublyLinkedList/doublyLinkedList.h"


//
// The time module calls a function after a specified delay. It will delay
// the call for at least the delay time. Please not that the callback can
// or will delayed due to the non-pre-emptive multitasking. To be more precise it 
// is only granteed that the call won't accoure before the timeout, so 
// never use this timer if you need an exact predictable timer.
//
// Alwasy keep in mind that adding a timer takes and the callback itself
// eats up lots of cpu time.
struct timerElement_t
{
  struct dlNode_t* prev;
  struct dlNode_t* next;  
  uint64_t timebomb; 
  void* callback;
};

void DebugTimerList();
void onInitTimer();
void onTimer();
void timerClearTimeout(struct timerElement_t* element);
struct timerElement_t* timerAddTimeout(uint16_t delay, 
    void (*callback)() );

#endif /*TIMER_H_*/
