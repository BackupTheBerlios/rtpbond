
#include "./timer.h"
#include "./../../toolkit/doublyLinkedList/doublyLinkedList.h"
#include "./../clock/clock.h"
#include "./../../hal/com/com1_debug.h"

#include <stdlib.h>
#include <avr\io.h>
#include <avr\interrupt.h>
#include <string.h>


// override the struct used in the doubly linked list
struct dlList_t timerList;


// delay in ms 16 bit -> max 1 Min...
// TODO ggf delay erhöhen (uint16_t -> uint32_t)

// Actually the delaytime are no real ms, as 1 sec would be equivalten to 1024
// which is damm close to 1000ms


struct timerElement_t* timerAddTimeout(uint16_t delay, 
    void (*callback)())
{


  struct timerElement_t* newNode
    = (struct timerElement_t*) malloc(sizeof(struct timerElement_t));





	//alx free --> timerClearTimeout
  
  // timestamp = now(in ms) + delay
  newNode->timebomb = clockGetTimeStamp()+delay;
  newNode->callback = callback;
 
  // If the list is empty, we can do a shortcut...
  if (timerList.firstNode == NULL)
  {
    insertDlNodeBeginning(&timerList,(struct dlNode_t*)newNode);    
    return newNode;
  }
  

  // ... otherise we have to add the entry into the sorted list
  // ... therefore we compare the timestamp...
  struct dlNode_t* node = timerList.firstNode;
 
  while (node != NULL)
  {
    if (((struct timerElement_t*)node)->timebomb > newNode->timebomb)
      break;
    node = node->next;
  }

  
  // We reached the end of the list without a hit?
  if (node == NULL)
  {
    insertDlNodeLast(&timerList,(struct dlNode_t*)newNode);    
    return newNode;
  }
  


  insertDlNodeBefore(&timerList,node,(struct dlNode_t*)newNode);

 
  return newNode;  
  
    
}


void DebugTimerList()
{


// If the list is empty, we can do a shortcut...
	if (timerList.firstNode == NULL)
	{
		putString_com1("\r\nTimer-Liste leer!!");
		return;
	}

	else
	{
		struct dlNode_t* node = timerList.firstNode;
		uint8_t anzahlTimer=0;
		char *x1=malloc(16);
		itoa(clockGetTimeStamp(),x1,10);

		putString_com1("\r\nclockGetTimeStamp:");
		putString_com1(x1);
 
		while (node != NULL)
		{	    
			anzahlTimer+=1;

			
			itoa(((struct timerElement_t*)node)->timebomb,x1,10);
			putString_com1("\r\nTB:");
			putString_com1(x1);
			node = node->next;
			
		}

		if (((struct timerElement_t*)(timerList.firstNode))->timebomb > clockGetTimeStamp())
			putString_com1("\r\ntimebomp groesser noch net erreicht:");
		else
		{
			putString_com1("\r\ntimebomp kleiner AUSSloesen:");
			onTimer();
		}

    	

		
		char *x2=malloc(2);

		itoa(anzahlTimer,x2,10);
		

		putString_com1("\r\nAnzahlTimer:");
		putString_com1(x2);

	}

}






// after any timeout or calling this function it is essential to NULL 
// the element variable
void timerClearTimeout(struct timerElement_t* element)
{
    
  removeDlNode(&timerList,(struct dlNode_t*)element);
  free(element);
}


void onTimer()
{
  if (timerList.firstNode == NULL)
    return;

  if (((struct timerElement_t*)(timerList.firstNode))->timebomb > clockGetTimeStamp())
    return;

//	putString_com1("\r\nAUSGELOESSSST:");
  
  
  // we need to remove the obsolete timer entry...
  // ... before we do the callback!
  void* callback = ((struct timerElement_t*)(timerList.firstNode))->callback ;
  timerClearTimeout(((struct timerElement_t*)(timerList.firstNode)));
  
  // as the entry is removed, we have to use...
  // ...our temporary variable to do the callback
  ((void(*) ()) (callback)) ();
  
  return; 
}

void onInitTimer()
{
  // Initalize Scheduler
  timerList.firstNode = NULL;
  timerList.lastNode = NULL;
}

