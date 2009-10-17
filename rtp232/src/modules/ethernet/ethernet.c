#include "./ethernet.h"
#include <stdlib.h>
#include <avr\io.h>
#include <avr\interrupt.h>
#include <string.h>

#include "./../arp/arp.h"
#include "./../ip/ip.h"
#include "../../config.h"
#include "./../../hal/com/com1_debug.h"

#include "./../../modules/clock/clock.h"




/* 
 * The Ethernet Layer is responsible for building, sending an receiving Ethernet Packets...
 */



/*struct macAddress_t soHardwareAddress 
   = { .octet1 = 0x00, .octet2 = 0x13, .octet3 = 0x02,
       .octet4 = 0x31, .octet5 = 0x91, .octet6 = 0x35 };*/

struct macAddress_t hEthernetAddress 
   = { .octet1 = highWord(MAC_GROUP1), 
       .octet2 = lowWord(MAC_GROUP1),
       .octet3 = highWord(MAC_GROUP2),
       .octet4 = lowWord(MAC_GROUP2), 
       .octet5 = highWord(MAC_GROUP3),
       .octet6 = lowWord(MAC_GROUP3) 
      };

typedef struct {
  struct macAddress_t mac;
} netEthernetEeprom_t;







//struct macAddress_t eeHardwareAddress EEMEM ;

// yes you realy get a pointer, this is for performance consideration
// never ever change the value of this pointer!
struct macAddress_t* getHardwareAddress()
{
  return &hEthernetAddress;
}

uint8_t onEthernetRequest(struct nicRequestHandle_t* handle, uint16_t type, 
                        struct macAddress_t* destMac)
{
	struct netEthernetHeader_t* header
	    = (struct netEthernetHeader_t*) nicAddPacketHeader(handle,sizeof(struct netEthernetHeader_t));
	
	header->destination = *destMac;
	header->source      = *(getHardwareAddress());
	header->etherType   = type;

	

	nicSendPacket(handle,0x00);
	
	nicFreeRequest(handle);


	
  return 0;
}
  
void onEthernetResponse(struct nicResponseHandle_t* handle)
{

// putString_com1("Ether-Empfangen");


  // drop tiny packets...
  if (nicResponseSize(handle) < sizeof(struct netEthernetHeader_t))
	  return;
	
  struct netEthernetHeader_t header;
  nicResponseRead(handle,(char*)&header,sizeof(struct netEthernetHeader_t));
	
  switch (header.etherType)
  {
    case ETHERTYPE_IP4: 
      onIPv4Response(handle,&(header.source));

//	  putString_com1("IP-Empfangen");

      break;
    case ETHERTYPE_ARP:

      onArpResponse(handle,&(header.source));
      break;
  }

  return;	
}                      

void onEthernet()
{
  struct nicResponseHandle_t* response
      = nicReceiveResponse();
        
  if (response == NULL)
    return;
    
  onEthernetResponse(response);
  nicFreeResponse(response);
}

void onInitEthernet()
{


  // define a struct for the configuration...
  netEthernetEeprom_t buf;

  // use eeprom settings if they are valid
  if ( EepromReadConfig((void*)MAC_EEPROM, &buf, sizeof(buf)) ) {
    hEthernetAddress = buf.mac;
  }
  // else hEthernetAddress keeps the fallback settings

  nicInitialize(&hEthernetAddress);
  
  onInitArp();
  onInitIPv4();  



  return;
}

#ifdef REQUIRES_RCFG

void ethernetEepromSetConfig(uint8_t * mac)
{
  netEthernetEeprom_t buf;
  buf.mac.octet1 = mac[0];
  buf.mac.octet2 = mac[1];
  buf.mac.octet3 = mac[2];
  buf.mac.octet4 = mac[3];
  buf.mac.octet5 = mac[4];
  buf.mac.octet6 = mac[5];

  EepromWriteConfig((void*)MAC_EEPROM, &buf, sizeof(buf));
}

void ethernetEepromGetConfig(uint8_t * mac)
{
  netEthernetEeprom_t buf;
  if ( !EepromReadConfig((void*)MAC_EEPROM, &buf, sizeof(buf)) ) {
    buf.mac = hEthernetAddress;
  }
  mac[0] = buf.mac.octet1;
  mac[1] = buf.mac.octet2;
  mac[2] = buf.mac.octet3;
  mac[3] = buf.mac.octet4;
  mac[4] = buf.mac.octet5;
  mac[5] = buf.mac.octet6;
}

#endif
