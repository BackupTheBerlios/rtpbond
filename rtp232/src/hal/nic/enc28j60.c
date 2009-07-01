#include "enc28j60.h"

struct {
  uint8_t bank;
} hEnc28j60;


void enc28j60_write(uint8_t command, const void* buffer, uint16_t length)
{
  
  // CS auf low ziehen -> bytes schreiben...
  PORTB &= ~(1<<PB0);

  //send the Command to the ENC:
  SPDR = command;

  while(!(SPSR & (1<<SPIF)));

  while(length--)
  {
    //send data
  	SPDR = *((uint8_t*)(buffer++));
	  while(!(SPSR & (1<<SPIF)));
  }

  // CS auf high
  PORTB |= (1<<PB0);
  
  return;
}

// generate some inline helper functions, which is pretty much the same 
// ... as a macro, except they can't cause syntax errors...
void enc28j60_write_byte(uint8_t command, uint8_t buffer)
{
  return enc28j60_write(command, &buffer, 1);
}


void enc28j60_read(uint8_t command, void* buffer, uint16_t length, uint8_t offset)
{

// mac und MII ben�tigen hier etwas spezielles...
// also prefix MA und MI

  // Pull down CS Pin
  PORTB &= ~(1<<PB0);

  //start read cmd:
  SPDR = command;
  //wait for completion
  while(!(SPSR & (1<<SPIF)));

  // Ignore dummy bytes...
  while(offset--) 
  {
    SPDR = 0x00;
    while(!((SPSR) & (1<<SPIF)));
  }

  while(length--)
  {    
    SPDR = 0x00;
    while(!((SPSR) & (1<<SPIF)));
    *((uint8_t*)(buffer++)) = SPDR;
  }

  // CS auf high
  PORTB |= (1<<PB0);
	
  return;
}


void enc28j60_select_bank(uint8_t address)
{

  // some registers are in all banks same...
  //... so a bank select would be useless
  if (address & ENC_MASK_BSEL_ALL)
    return;

  // the same applies if the bank is...
  // ... already selected
  if (hEnc28j60.bank == (address & ENC_MASK_BSEL))
    return;

  // ! DO NOT CALL enc28j60_crbfc or bfs this will end up
  //   in an infinite loop...

  // clear bank select bits
  enc28j60_write_byte(
      ENC_OP_BFC | ENC_REG_ECON1, 
      (ENC_REG_ECON1_BSEL0 | ENC_REG_ECON1_BSEL1) );

  // set the bank select bits
  enc28j60_write_byte(
      ENC_OP_BFS | ENC_REG_ECON1, 
      (address & ENC_MASK_BSEL) >> 5 );

  hEnc28j60.bank = (address & ENC_MASK_BSEL);

  return;
}



void enc28j60_init()
{
 
  // Als erstes den SPI Bus aktivieren

  // Set MOSI, SS and SCK output, all others input
  //DDRB = (1<<DDB2)|(1<<DDB1) | (1 << PB4);
  DDRB = (1<<PB2 ) | (1<<PB1) | (1<<PB0);

  // MISO = IN
  DDRB &= ~(1<<PB3); 

  // Pull up Chipselect to high...
  PORTB |= (1<<PB0);

  // Enable SPI, Master, set clock rate 
  SPCR = (0<<CPOL)|(1<<MSTR)|(0<<DORD)/*|(0<<SPR1)|(0<<SPR0)*/|(1<<SPE);

  // Double SPI clock
  SPSR = (1<<SPI2X);

  // TODO: Werden structs nicht automatisch genullt?
  hEnc28j60.bank = 0;

  return;
}



/******************************************************************************************

  Control Register Read functions...

******************************************************************************************/

// the MI and MA register have an  offset of 1 Byte while reading
// this function determins the offset which should be used.
// as the previous function this is inline ...
uint8_t enc28j60_calcOffset(uint8_t address)
{
  // Bei den MI und MA Registern muss ein Dummybyte gelesen werden
  if ( ((address & ENC_MASK_BSEL) == ENC_BSEL_2) 
        && ((address & ENC_MASK_REGADR) <= ENC_REG_MIRDH))
  {
    return 1;
  }

  if ( ((address & ENC_MASK_BSEL) == ENC_BSEL_3) 
        && ((address & ENC_MASK_REGADR) <= ENC_REG_MAADR4))
  {
    return 1;
  }
        
  if ( ((address & ENC_MASK_BSEL) == ENC_BSEL_3) 
              || ((address & ENC_MASK_REGADR) == ENC_REG_MISTAT))
  {
    return 1;
  }

  return 0;  
}

// This function is beeing called from the crget* functions.
// It is inline in order to speedup calling and saving stack memeory
void* enc28j60_rcr2(uint8_t address,  void* buffer, uint16_t len)
{
  // switch to the corresponding memory bank
  enc28j60_select_bank(address);  
  
  // ... then start reading...
  enc28j60_read(ENC_OP_RCR | (address & ENC_MASK_REGADR), buffer, len, enc28j60_calcOffset(address));
  
  // now return pointer to the buffer for better usability
  return buffer;
}


uint8_t enc28j60_crgetc(uint8_t address)
{
  uint8_t buffer;
  enc28j60_rcr2(address,&buffer,1);
  return buffer;
}

uint16_t enc28j60_crgetw(uint8_t address)
{
  uint16_t buffer;
  enc28j60_rcr2(address,&buffer,2);
  return ((buffer &0x00FF) << 8) | ((buffer&0xFF00) >> 8);
}

void* enc28j60_crgets(uint8_t address, void* buffer, uint16_t len)
{
  enc28j60_rcr2(address,buffer,len);
  return buffer; 
}

/******************************************************************************************

  Control Register Write functions...

  These functions provide a way to manipulate the Control registers.
  They appear to be very tiny wrapper functions. But they call several inlined function.
  This incleases the size dramatically but improves the speed significately.

******************************************************************************************/

// Write functions 
//  this function is explicitly declared inline in order to speedup calls
void enc28j60_wcr(uint8_t address, const void* buffer, uint16_t len)
{
  // Speicherbank ausw�hlen...
  enc28j60_select_bank(address);
  // ... und die Daten schreiben 
  enc28j60_write(ENC_OP_WCR | (address & ENC_MASK_REGADR),buffer, len);

  return;
}

// write a single control register...
void enc28j60_crputc(uint8_t address, uint8_t buffer)
{
  enc28j60_wcr(address, &buffer, 1);
  return;
}

// write a word into a control registers (usefull for enc pointers)...
// ... the endianess is corrected!
void enc28j60_crputw(uint8_t address, uint16_t buffer)
{
  // correct the endianness, swap high an low byte
  // buffer = (((uint16_t)buffer &0x00FF) << 8) | (((uint16_t)buffer&0xFF00) >> 8);
  enc28j60_wcr(address, &buffer, 2);
  return;
}

void enc28j60_crputs(uint8_t address, const void* buffer, uint16_t len)
{
  enc28j60_wcr(address, buffer, len);
  return;
}

/******************************************************************************************

  Control Register Bit Manipulation functions...

  These functions provide a way to manipulate the Control registers.
  They appear to be very tiny wrapper functions. But they call several inlined function.
  This incleases the size dramatically but improves the speed significately.

******************************************************************************************/

// Bit Field Set
void enc28j60_crbfs(uint8_t address, char flags)
{
  enc28j60_select_bank(address);
  enc28j60_write(ENC_OP_BFS | (address & ENC_MASK_REGADR),&flags,1);
  return;
}

// Bit Field Clear
void enc28j60_crbfc(uint8_t address, char flags)
{
  enc28j60_select_bank(address);
  enc28j60_write(ENC_OP_BFC | (address & ENC_MASK_REGADR),&flags,1);
  return;
}


/******************************************************************************************

  Buffer Memory Read functions...

  These functions provide a way to manipulate the Buffer Memory of the ENC.
  They appear to be very tiny wrapper functions. But they call several inlined function.
  This increases the size dramatically but improves the speed significately.

******************************************************************************************/


// Read a char from Buffer Memory
uint8_t  enc28j60_bmgetc()
{
  uint8_t buffer;
  enc28j60_read(ENC_OP_RBM ,&buffer, 1,0);
  return buffer;
}
// Read a word from Buffer Memory
uint16_t enc28j60_bmgetw()
{
  uint16_t buffer;
  enc28j60_read(ENC_OP_RBM ,&buffer, 2,0);
  return buffer;
}
// Read a string from Buffer Memory
void* enc28j60_bmgets(void* buffer, uint16_t len)
{
  enc28j60_read(ENC_OP_RBM ,buffer, len,0);
  return buffer;
}


/******************************************************************************************

  Buffer Memory Write functions...

  These functions provide a way to manipulate the Buffer Memory of the ENC.
  They appear to be very tiny wrapper functions. But they call several inlined function.
  This increases the size dramatically but improves the speed significately.

******************************************************************************************/



// Write Buffer Register, the byte order is not swaped!
void enc28j60_bmputs(const void* buffer, uint16_t len)
{
  enc28j60_write(ENC_OP_WBM,buffer, len);
  return;
}

// Soft Reset
void enc28j60_reset()
{ 
  enc28j60_write(0xFF,0,0);
  return;
}

// Write phy register
void enc28j60_prputw(uint8_t address, uint16_t buffer)
{
  enc28j60_crputc(ENC_REG_MIREGADR,address);

  enc28j60_crputc(ENC_REG_MIWRL, (buffer&0xFF));
  enc28j60_crputc(ENC_REG_MIWRH,  (buffer>> 8));

  while (enc28j60_crgetc(ENC_REG_MISTAT) & ENC_REG_MISTAT_BUSY);

  return;
}
