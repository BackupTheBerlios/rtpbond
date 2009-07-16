#include "./nic.h"
#include <util/delay.h>
#include <stdlib.h>

#include "./../com/com1_debug.h"


struct soPacket_t
{
  struct {
    void* data;
    uint16_t length;
  } header;    
  struct soPacket_t *payload;
  uint16_t length;

};

struct nicRequestHandle_t* nicNewRequest()
{
  struct nicRequestHandle_t* handle;
  handle = (struct nicRequestHandle_t*) malloc(sizeof(struct nicRequestHandle_t));
  
  handle->packet = NULL;

  return handle;  
}


void* nicAddPacketHeader(struct nicRequestHandle_t* handle, uint16_t len)
{

  // construct the encapsulating packet/frame
  
  // first get memory for the packet/frame...
  struct soPacket_t *packet;
  packet = (struct soPacket_t*) malloc(sizeof(struct soPacket_t));

  // ... then for the nested header
  packet->header.length = len; 
  if (len == 0 )
    packet->header.data = NULL;
  else 
    packet->header.data = (char*)malloc(len);
  

  // now set and/or update the payload...
  packet->payload = handle->packet;
   
  if (packet->payload == NULL)
    packet->length = packet->header.length;
  else
  	packet->length  = packet->header.length + packet->payload->length;
  
  // now update the packet handle
  handle->packet = packet;
  
  // everything done, so return the header 
  return packet->header.data;  
}



uint16_t nicGetPacketSize(const struct nicRequestHandle_t* handle)
{
  if (handle->packet == NULL)
    return 0;

  return handle->packet->length;
}

void nicWaitForSend()
{
  //still sending ?
  uint8_t i = 50;
  while( i-- )
  {
    // finished sending?
    if ( ! (enc28j60_crgetc(ENC_REG_ECON1) & ENC_REG_ECON1_TXRTS))
      return;
        
    // ... no - let's wait some more ms...  
    _delay_ms(10);    
  }

  putString_com1("Waitedlong enough");
  
  // ...we waited long enough, continue anyway
  return;
}

uint8_t nicSendPacket(const struct nicRequestHandle_t* handle, uint8_t flags)
{
  // Wait until we are ready to send...
  nicWaitForSend();

  // setup tx buffer start
  enc28j60_crputw(ENC_REG_ETXSTL,ENC_ETXST);
  //enc28j60_crputc(ENC_REG_ETXSTL,0x00);
  //enc28j60_crputc(ENC_REG_ETXSTH,0x1A);

  //setup write pointer
  enc28j60_crputw(ENC_REG_EWRPTL,ENC_ETXST);
  //enc28j60_crputc(ENC_REG_EWRPTL,0x00);
  //enc28j60_crputc(ENC_REG_EWRPTH,0x1A);

  // write control block
  uint8_t dummy = 0;
  enc28j60_bmputs(&dummy,1);
  
  // now start building the packet...
  
  struct soPacket_t* packet = handle->packet;
 
       
  while( packet != NULL)
  {    
    enc28j60_bmputs(packet->header.data,packet->header.length);   
    packet = packet->payload;
  }
  
  // calculate the Packet length...
  enc28j60_crputw(ENC_REG_ETXNDL, ENC_ETXST + handle->packet->length);
  //enc28j60_crputc(ENC_REG_ETXNDL,0x4E);  
  //enc28j60_crputc(ENC_REG_ETXNDH,0x1A);
  
	// according to the Silicon Errata Document (10. Transmit Logic) ...
  // .. we need to reset the transmit logic...
  enc28j60_crbfs(ENC_REG_ECON1,ENC_REG_ECON1_TXRST); 
  enc28j60_crbfc(ENC_REG_ECON1,ENC_REG_ECON1_TXRST);
  
  // transmit request...
  enc28j60_crbfs(ENC_REG_ECON1,ENC_REG_ECON1_TXRTS | ENC_REG_ECON1_RXEN);

  // this is now debug only...
  // ... 
  
  if (flags & NIC_PACKET_SEND_NOWAIT)
    return 0;
  
  // wait until transmit is completed...
  while ( ! (enc28j60_crgetc(ENC_REG_ECON1) & ENC_REG_ECON1_TXRTS) );
  
  if (flags & NIC_PACKET_SEND_NOERRCHK)
    return 0;

  // check ESTAT for send Errors...
  if ((enc28j60_crgetc(ENC_REG_ESTAT) 
         & (ENC_REG_ESTAT_TXABRT | ENC_REG_ESTAT_LATECOL)) == 0 )
    return  0;
 
  // we have an error, we might want to check the Send Vector...      
  //setup read pointer
  // TODO Parse Send Vector...
  
  // Backup read pointer...
  
  // ...Read Vector...
  
  // ...Restore read pointer
  
  // parse Vector...
  
  
  return 1;  
}


void nicFreeRequest(struct nicRequestHandle_t* handle)
{
  if (handle == NULL)
    return;
	  
  struct soPacket_t *packet
      = handle->packet; 
       
  while(packet != NULL)
  {
    // free header if applicable
    if (packet->header.data != NULL)
      free(packet->header.data);
    
    // update the handle...
    handle->packet = packet->payload;
    // then free the packet...
    free(packet);
    packet = handle->packet;             
  }  

  // finally free the handle itself...
  free(handle); 
  
  return;
}

void nicInitialize(struct macAddress_t* mac)
{
  //TODO Change to Full Duplex Settings...
  enc28j60_init();
  
  enc28j60_reset();
  _delay_ms(50);

  // Let's wait until the ENC28J60 is ready...
  while( (enc28j60_crgetc(ENC_REG_ESTAT) & ENC_REG_ESTAT_CLKRDY) == 0 );
  
  // Setup RX buffers...
  enc28j60_crputc(ENC_REG_ERXSTL, 0x00);
  enc28j60_crputc(ENC_REG_ERXSTH, 0x00);

  enc28j60_crputc(ENC_REG_ERXNDL, 0xFF);
  enc28j60_crputc(ENC_REG_ERXNDH, 0x19);

  // Setup TX Buffers...
  enc28j60_crputw(ENC_REG_ETXSTL, ENC_ETXST);
  //enc28j60_crputc(ENC_REG_ETXSTL, 0x00);
  //enc28j60_crputc(ENC_REG_ETXSTH, 0x1A);

  enc28j60_crputw(ENC_REG_ETXNDL, ENC_ETXND);
  //enc28j60_crputc(ENC_REG_ETXNDL, 0xFF);
  //enc28j60_crputc(ENC_REG_ETXNDH, 0x1F);

  // Clear MARST in MACON2 to start Mac Module 
  enc28j60_crputc(ENC_REG_MACON2, 0x00);

  // enabled MARXEN in MACON1 to enable receive ...
  // ...TXPAUS and RXPAUS for FullDuplex with FlowControl
  enc28j60_crputc(
      ENC_REG_MACON1, 
      ENC_REG_MACON1_MARXEN | ENC_REG_MACON1_RXPAUS | ENC_REG_MACON1_TXPAUS);

  // enable automatic Padding (PADCFG) automatic CRC ...
  // .. (TXCRCEN) and Framelength Check in MACON3
  enc28j60_crputc(
      ENC_REG_MACON3,
      ENC_REG_MACON3_PADCFG0 | ENC_REG_MACON3_TXCRCEN  | ENC_REG_MACON3_FRMLEN);

  // set Maximum frame length
  // Programm MAMXFL regiserts to set maximum framelength. 1518 (0x5EE) are normal
  enc28j60_crputc(ENC_REG_MAMXFLL,0xEE);
  enc28j60_crputc(ENC_REG_MAMXFLH,0x05);

  // TODO ENC auf Full duplex setzen...
  // Set MABBIPG with 15h (recommended for Full Duplex)
  enc28j60_crputc(ENC_REG_MABBIPG, 0x12);

  // set MAIPGL to 12h
  enc28j60_crputc(ENC_REG_MAIPGL, 0x12);

  // set HAIPGH to 0h
  enc28j60_crputc(ENC_REG_MAIPGH, 0x0C);

  
  //10.) programm mac address: BYTE BACKWARD !
  enc28j60_crputc(ENC_REG_MAADR0,mac->octet6);
  enc28j60_crputc(ENC_REG_MAADR1,mac->octet5);
  enc28j60_crputc(ENC_REG_MAADR2,mac->octet4);
  enc28j60_crputc(ENC_REG_MAADR3,mac->octet3);
  enc28j60_crputc(ENC_REG_MAADR4,mac->octet2);
  enc28j60_crputc(ENC_REG_MAADR5,mac->octet1);

  // Initialize Phy
  // TODO PHY flags über define festlegen...
  // disable loopback
  enc28j60_prputw(0x10,(1 << 8));

  // enable LEDs
  enc28j60_prputw(0x14,0x0472);

  // enable receive
  enc28j60_crbfs(ENC_REG_ECON1,ENC_REG_ECON1_RXEN);
  
  // set next packet pointer to 0
  enc28j60_crputw(ENC_REG_ERDPTL,0);

  return;
}

struct nicResponseHandle_t* nicReceiveResponse()
{  

  // Check the Interruptflags for new Packkets...
  if ((enc28j60_crgetc(ENC_REG_EIR) & ENC_REG_EIR_PKTIF ) == 0)
    //... and then consult the packet counter (due to bad silicon) 
    if (enc28j60_crgetc(ENC_REG_EPKTCNT) == 0)
      return NULL;
 
  // ... generate a handle with a dummy Packet header...
  
  struct nicResponseHandle_t* handle 
      = (struct nicResponseHandle_t*) malloc(sizeof(struct nicResponseHandle_t));
  
  // ...now set and update the readpointer  
  handle->nextPacketPtr = enc28j60_bmgetw();  
  
  //update the length...
  handle->length = enc28j60_bmgetw();

  // ignore the receive Vector   
  enc28j60_bmgetw();
  
  return handle;  
}

void nicFreeResponse(struct nicResponseHandle_t* response)
{
  
  //fix Packet Pointer (see Errata)
  if (((response->nextPacketPtr)-1 < ENC_ERXST) || ((response->nextPacketPtr)-1 > ENC_ERXND)) 
    // TODO: WIRKLICH ENC_ERXND?
    enc28j60_crputw(ENC_REG_ERXRDPTL,ENC_ERXND);
  else
    enc28j60_crputw(ENC_REG_ERXRDPTL,(response->nextPacketPtr)-1); 

  //decrement packet counter:
  enc28j60_crbfs(ENC_REG_ECON2,ENC_REG_ECON2_PKTDEC);
  
  //update the next packet pointer
  enc28j60_crputw(ENC_REG_ERDPTL,(response->nextPacketPtr));  
  
  free(response);
  
  return;
}

uint16_t nicResponseRead(struct nicResponseHandle_t* response, char* buffer, uint16_t len)
{
  // adjust the length, if it is bigger than the received packet...
  if (response->length < len)
    len = response->length;
         
  // read enc packet...
  enc28j60_bmgets(buffer,len);
  
  // ...decrement length
  response->length -= len;

  return len;  
}

uint16_t nicResponseSize(struct nicResponseHandle_t* response)
{
  if (response == 0)
    return 0;
   
  return response->length;
}



