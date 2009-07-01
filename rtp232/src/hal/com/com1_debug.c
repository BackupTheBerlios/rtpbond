#ifdef REQUIRES_SERIAL




#include "./com1_debug.h"
#include <avr/interrupt.h>
#include "./../../toolkit/queue/queue.h"
#include "./../../modules/timer/timer.h"
#include "./../../modules/rtp/rtp.h"

#include "./../../modules/rcfg/rcfg.h"

#include <string.h>


#define 	BAUD 	115200


volatile extern uint16_t menge2;


volatile extern uint16_t debug;


struct StructCommandBuffer CommandBuffer;




struct 
  {
    // define our buffer...
     char recvBuffer[40];
    // ... and a helper, which refers to the next empty char
     char* recvCount;     
  
  }
  com1_recv;




//------------------------------------------------------------------------
// Initialisierungen
//------------------------------------------------------------------------
void com1Initialize(uint16_t baud, uint8_t settings)
{

	DDRD |= (1<<PD2);	//PD2 =0 eingang (RXD)		0100
	DDRD &= ~(1<<PD3);		//PD3 =1 ausgang (TXD)



	PORTD |= (1<<PD2);	//PD0 ist eingang --> pullup hoch

	

	

	// UART initialisieren

 // disable double speed, as we don't need it 
    UCSR1A &= ~(1 << U2X0);

	UBRR1L=(uint8_t)(F_CPU/(BAUD*16L))-1; 	// Baudrate festlegen
	UBRR1H=(uint8_t)((F_CPU/(BAUD*16L))-1)>>8; 	// Baudrate festlegen

//	UBRR1L=7; 	// Baudrate festlegen
//	UBRR1H=7>>8; 	// Baudrate festlegen


	UCSR1B =(1<<RXCIE)|(1<<TXCIE)|(1<<UDRIE)|(1<<RXEN)|(1<<TXEN);
	UCSR1C =(1<<UCSZ1)|(1<<UCSZ0);


	TCCR0=0x05; 	// Teiler 1/1024
	TIMSK|=1; 	// Interrupt bei Überlauf

com1_recv.recvCount = com1_recv.recvBuffer;

}

//----------------------------------------------------------------------
// Titel 	: C-Funktion Zeichen zu UART senden.
//----------------------------------------------------------------------
// Funktion 	: ...
// IN  	 	: char data
// OUT 	 	: ...
//----------------------------------------------------------------------
void putChar(char data)
{
	//warte bis UDR leer ist UCSRA / USR bei z.B.: 2313
	while (!(UCSR1A&32));
	//sende
	UDR1=data;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Titel 	: C-Funktion Zeichen von UART holen.
//----------------------------------------------------------------------
// Funktion 	: ...
// IN  	 	: ...
// OUT 	 	: data
//----------------------------------------------------------------------
char getChar()
{
	//warte bis RX-complete RXC UCSRA / USR bei z.B.: AT090S2313
	while (!(UCSR1A&128));
	//empfangen
	;

	return UDR1;
}

//--------------------------------------------------------------------
// USART_UDRE_vect - UART Interrupt bei Senderegister leer
//--------------------------------------------------------------------
ISR(USART1_UDRE_vect)
{
	UCSR1B &= ~ (1 << UDRIE1);
}

//--------------------------------------------------------------------
// USART_TXC_vect - UART Interrupt bei Sendevorgang beendet
//--------------------------------------------------------------------
ISR(USART1_TX_vect)
{
	
}



//--------------------------------------------------------------------
// USART_RXC_vect - UART Interrupt bei Datenempfang komplett
//--------------------------------------------------------------------
ISR(USART1_RX_vect)
{


	char x=getChar();
	putChar(x);


	if(x != 0xD)	//einfaches newline testen
	{
		
		// Save the received byte...
	    com1_recv.recvCount[0] = x;
  
	    // increase our helper...
	    com1_recv.recvCount++;
		
	}
	else 
	{

		com1_recv.recvCount[0] = '\0';

		ExecuteCommand(com1_recv.recvBuffer,com1_recv.recvCount-com1_recv.recvBuffer);
	
	
		putString("nix\r\n");

		com1_recv.recvCount = com1_recv.recvBuffer;

	
	}

}


void ExecuteCommand(char* command,uint8_t len)
{
	if(strcmp(command,"hallo")==0)
	{
		putString("Befehl erfolgreich dekodiert");
		debug=0;
		DebugTimerList();
	}
	else if(strcmp(command,"nicht")==0)
	{
		putString("Befehl2 erfolgreich dekodiert");
		debug=1;
	}

	else
	{
	 	 CommandBuffer.NewCommand=1;
		strcpy(CommandBuffer.recvCommand,command);
		CommandBuffer.len=len;
	}

}






//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Titel 	: C-Funktion Zeichenkette zu UART senden.
//----------------------------------------------------------------------
// Funktion 	: ...
// IN  	 	: char *buffer, Zeichenkette mit NUll abgeschlossen
// OUT 	 	: ...
//----------------------------------------------------------------------
void putString(char *buffer)
{
int i=0;
	for (i=0; buffer[i] !=0;i++) putChar (buffer[i]);
}




#endif
