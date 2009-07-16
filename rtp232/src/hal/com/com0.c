#ifdef REQUIRES_SERIAL


#include "./com0.h"

#include <avr/interrupt.h>
#include "./../../toolkit/queue/queue.h"
#include "./../../modules/timer/timer.h"
#include "com1_debug.h"
#include "./../../modules/rcfg/rcfg.h"
#include "./../../modules/rtp/rtp.h"
#include "./../../config.h"


extern struct StructCommandBuffer CommandBuffer;


  /*
   *  UBRR settings for 14.7456 MHz 
   *   
   *   Baud  | U2X=0 | U2X=1 | 
   * --------------------------  
   *   2400  |   383 |   767 |
   *   4800  |   191 |   383 |
   *   9600  |    95 |   191 |
   *   14.4k |    63 |   127 |
   *   19.2k |    47 |    95 |
   *   28.8k |    31 |    63 |
   *   38.4k |    23 |    47 |
   *   57.6k |    15 |    31 |
   *   76.9k |    11 |    23 |
   *  115.2k |     7 |    15 |
   *  230.4k |     3 |     7 |
   *  250.0k |     3 |     6 | *
   *    0.5M |     1 |     3 | *
   *    1.0M |     0 |     1 | *
   */


    
  struct 
  {
    // define our buffer...
    volatile char recvBuffer[COM0_MAX_RX_BUF];
    // ... and a helper, which refers to the next empty char
    volatile char* recvCount;     
  
    volatile char sendBuffer[COM0_MAX_TX_BUF];
    volatile char* sendCount;

	volatile uint8_t SendBufferSizeLimit;
  
    struct queue_t recvFiFo;   
    struct queue_t sendFiFo;
    
    struct timerElement_t* timeout;    
  }
  hUsart0;
  
  struct queueElement_t
  {
    struct queueNode_t* prev;    
    char* buffer;
    uint8_t len;
  };




  void com0RotateRecvBuffer()
  {
    // We should rotate the Recv Buffers, therefore we have to disable...
    // ...the RX interrupt. If a interrupt would fire, while we rotate the
    // ... Buffer, we would mostlikely endup in a crash due to illeagal... 
    // ... memory allocations.
    UCSR0B &= ~(1 << RXCIE0);


    
    // Reject adding to queue, if it is full..
    if (queueIsFull(&hUsart0.recvFiFo) == 0)
    {


      // create a new queue element
      struct queueElement_t* node 
        = (struct queueElement_t*) malloc(sizeof(struct queueElement_t));

    
      // ... if the buffer is empty we can do a short cut...
      if (hUsart0.recvCount != hUsart0.recvBuffer)
      {
        // calculate the length
        uint8_t len = hUsart0.recvCount-hUsart0.recvBuffer;
      
        node->len = len;
        node->buffer = (char*) malloc(len);
      
        // copy buffer
        while (len--)
        {
          node->buffer[len] = hUsart0.recvBuffer[len];
        }      

      }
      else
      {
        // thats the short cut for empty buffers
        node->len = 0;
        node->buffer = NULL;

		//here we send an iamalive paket over config channel too
		//but only if there is not stored an commandanswer already

		if(CommandBuffer.NewCommand==0)
		{
			rtpSendCommand("iamalive", 0, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);
		}


      }
     
    	
    
      // enqueue the recevived data
      pushQueueEntry(&hUsart0.recvFiFo,(struct queueNode_t*)node);


	  
    }//if(queu is full)
	else
	{
		putString_com1("achtung zu vile daten");
	}
      
      // ... finally don't forget to reset our helper to the base address
    hUsart0.recvCount = hUsart0.recvBuffer;
    
    // As we are done with the RX Buffer it's now save to reenable the interrupt
    UCSR0B |= (1 << RXCIE0);

  }
  
  void com0RotateSendBuffer()
  {
    
    /* !!! IF YOU MOFIFY THIS BLOCK ALWAYS SYNC IT WITH THE CODE !!! */ 
    /* !!! IN com0SendBytes()                                    !!! */
    
    // in order to ayoid timing issues, disable TX Interrupt
    UCSR0B &= ~ (1 << UDRIE0);
   
    struct queueElement_t* node
      = (struct queueElement_t*) popQueueEntry(&hUsart0.sendFiFo);
    
    // we use top aligned blocks, this saves lots of processing time...
    // ... but makes live more complicated
    uint8_t len = node->len;
    char* worker = (char*) (hUsart0.sendBuffer+COM0_MAX_TX_BUF-len);    
    
    while (len--)
    {
      worker[len] = node->buffer[len];
    }   

    hUsart0.sendCount = hUsart0.sendBuffer+COM0_MAX_TX_BUF-len;
    
    if (node->len > 0) 
      free(node->buffer);
    free(node);
  
    // reenable TX Interrupts...
    UCSR0B |= (1 << UDRIE0);
  }
  
  
  ISR (USART0_RX_vect)
  {

	

	while (!(UCSR0A&32));
	char x=UDR0;
    // Save the received byte...
    hUsart0.recvCount[0] = x;


    // increase our helper...
    hUsart0.recvCount++;
    
    // ... and check, if we need to rotate the buffer
    if (hUsart0.recvCount < (hUsart0.recvBuffer+hUsart0.SendBufferSizeLimit))
      return;
  

   putString_com1("rot");
    com0RotateRecvBuffer();
    
    return;
  }

  ISR (USART0_UDRE_vect )
  {
    // we actually do here something nasty...
    // ... we are referencing against the top address instead of the base address
    UDR0 = hUsart0.sendCount[0];
    
    // check if we need to rotate buffers...
    if (hUsart0.sendCount < (hUsart0.sendBuffer+COM0_MAX_TX_BUF-1))
    {
      
      hUsart0.sendCount++;
      return;
    }
      
    // nothing to rotate?
    if (queueIsEmpty(&hUsart0.sendFiFo))    
    {
      // so disable the send interrupt...
      UCSR0B &= ~ (1 << UDRIE0);
      return;
    }
   
    
    com0RotateSendBuffer();
        
    return;
  }    
  
  void onCom0Timeout()
  {



    // If we are here, we obviously have to rotate the buffers.

	
    com0RotateRecvBuffer();


        
    //Keep the timer up and running...
    hUsart0.timeout = timerAddTimeout(COM0_TIMEOUT,onCom0Timeout);
	
  }
  
  void com0Initialize(uint16_t prescaler, uint8_t parity, uint8_t stop_bits, uint8_t data_bits,uint8_t SendBufferSizeLimit)
  {
    
	//init USART0 ( for data)

   	    // TX0 = OUT;
    DDRE = (1<<PE1 );
  
    // RX0 = IN
    DDRE &= ~(1<<PE0);

    /* Set baud rate */
    UBRR0H = (unsigned char)(prescaler>>8);
    UBRR0L = (unsigned char)prescaler;
    
    // disable double speed, as we don't need it 
    UCSR0A &= ~(1 << U2X0);
    // Enable receiver and transmitter module, as well as RX Interrupt..
    // ... tx interrupt will be activated when needed.    
    UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1 << RXCIE0) /*| (1 << UDRIE0)*/;



	//das muss geändert werden
    /* Set frame format: 8data, 2stop bit */


	UCSR0C |= (parity<<UPM0);
    UCSR0C |= (stop_bits<<USBS0);
	UCSR0C |= (data_bits<<UCSZ0);
    

    
    hUsart0.recvFiFo.first = NULL;
    hUsart0.recvFiFo.last  = NULL;
    hUsart0.recvFiFo.elements = 5;    
    hUsart0.recvCount  = hUsart0.recvBuffer;
    
    hUsart0.sendFiFo.first = NULL;
    hUsart0.sendFiFo.last  = NULL;
    hUsart0.sendFiFo.elements = 5;
    hUsart0.sendCount  = hUsart0.sendBuffer+COM0_MAX_TX_BUF;

	hUsart0.SendBufferSizeLimit=SendBufferSizeLimit;
    
    hUsart0.timeout = timerAddTimeout(COM0_TIMEOUT,onCom0Timeout);
    
    return;
  }





//----------------------------------------------------------------------
// Titel 	: C-Funktion Zeichen zu UART senden.
//----------------------------------------------------------------------
// Funktion 	: ...
// IN  	 	: char data
// OUT 	 	: ...
//----------------------------------------------------------------------
void com0putChar(char data)
{
	//warte bis UDR leer ist UCSRA / USR bei z.B.: 2313
	while (!(UCSR0A&32));
	//sende
	UDR0=data;
}



//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Titel 	: C-Funktion Zeichenkette zu UART senden.
//----------------------------------------------------------------------
// Funktion 	: ...
// IN  	 	: char *buffer, Zeichenkette mit NUll abgeschlossen
// OUT 	 	: ...
//----------------------------------------------------------------------
void com0putBuffer(char *buffer,uint8_t len)
{
int i=0;
	for (i=0; i<len ;i++) com0putChar (buffer[i]);
}




  // sends the bytes from the buffer...
  // ... the buffer copied, so it is save to free it any time
  void com0SendBytes(char* buffer, uint8_t len)
  {
    
/*    // drop if packet length ist out of bounds or if it's an empty packet
    if ((len > COM0_MAX_TX_BUF) || (len == 0))
      return; 
    
    // TX Interrupt running?
    if (UCSR0B & (1 << UDRIE0))
    {
      // abort, if queue is full
      if (queueIsFull(&hUsart0.sendFiFo) == 1)
        return;
      
      // create queue element
      struct queueElement_t* node
        = (struct queueElement_t*) malloc(sizeof(struct queueElement_t));
         
      node->len = len;
      node->buffer = (char*) malloc(len);
         
      // copy buffer
      while (len--)
      {
           node->buffer[len] = hUsart0.sendBuffer[len];
      }
      
      pushQueueEntry(&hUsart0.sendFiFo,(struct queueNode_t*)node);

	  

      return;
    }
    
    //no Interrupt -> no TX queue empty -> do a short cut...
  
    // the helper points to our base address of the top aligned memory
    char* helper = (char *) (hUsart0.sendBuffer+COM0_MAX_TX_BUF-len);
    while (len--)
    {
      helper[len] = buffer[len];
    }
  
    hUsart0.sendCount = helper;
    
    UCSR0B |= (1 << UDRIE0);

	*/
      
    return;
  }
  
  // reveives bytes, any reveived bytes will be written to the buffer...
  // ... memory will be automatially allocated memory, therefore make sure
  // that your free it!;
  void com0RecvBytes(char** buffer, uint8_t* len)
  {
	


    struct queueElement_t* node
      = (struct queueElement_t*) popQueueEntry(&hUsart0.recvFiFo);


    *len = node->len;   
    *buffer = node->buffer;     
    
    free(node);
     
    // Reset timeout as packet has been most recently read
    // so reschedule the timeout
    // TODO: Etwas suboptimal, in onCom0Timeout wird der timer auch
    // neu gesetzt. Und auf einen onCom0Timeout wird in den meisten
    // fï¿½llen auch ein com0RecvBytes aufruf folgen...


    timerClearTimeout(hUsart0.timeout); //alx EINGEFÜGT	


    hUsart0.timeout = timerAddTimeout(COM0_TIMEOUT,onCom0Timeout); 



	
    return;
  }
  
  uint8_t com0HasBytes()
  {
    return queueIsEmpty(&hUsart0.recvFiFo);
  }
  
  
  
#endif
