#include "./arp.h"
#include "./../ip/ip.h"
#include "./../../toolkit/netlib/netlib.h"
#include "./../timer/timer.h"  
#include <stdlib.h>

#define ARP_OPCODE_REQUEST  0x0100
#define ARP_OPCODE_RESPONSE 0x0200


// the arpcache has to bes static, otherwise it would fragment 
// our memeory...
struct arpCacheEntry_t
{
  // used to store data...
  struct macAddress_t mac;
  ipAddress_t ip;
  uint8_t age;
};

struct 
{
  struct arpCacheEntry_t arpCache[ARP_CACHE_SIZE];
  uint8_t timestamp;
}hArp;


struct soArpHeader_t
{
  uint16_t htype;            // Ethernet = 1;
  uint16_t ptype;            // IP = 0x0800;
  uint8_t  hlen;             // length in bytes of a hardware address, usually 6
  uint8_t  plen;             // length in bytes of a logical/protocol address, for IPv4 use 4
  uint16_t operation;             // 1 for request, 2 for reply
  struct macAddress_t sha; // sender hardware address
  ipAddress_t spa;             // sender protocol address
  struct macAddress_t tha; // target hardware address, needs to be 0 on requests
  ipAddress_t tpa;             // target protocol address
};

uint8_t arpSendRequest(ipAddress_t* tpa)
{  
  struct nicRequestHandle_t* handle = nicNewRequest();    
    
  return onArpRequest(handle,tpa,NULL);
}

uint8_t arpSendResponse(ipAddress_t* tpa,  struct macAddress_t* tha)
{  
  struct nicRequestHandle_t* handle = nicNewRequest();
  
  return onArpRequest(handle,tpa,tha);
}

uint8_t onArpRequest(struct nicRequestHandle_t* handle,ipAddress_t* tpa, struct macAddress_t* tha)
{
  struct soArpHeader_t* header 
    = (struct soArpHeader_t*) nicAddPacketHeader(handle, sizeof(struct soArpHeader_t));

  // Set Hardwaretype to Ethernet...
  header->htype = 0x0100;
  // ... the protocol is IP
  header->ptype = 0x0008;
  // Ethernet addresses are 6bytes long...
  header->hlen  = 0x06;
  // ... ip adresses are 4bytes.
  header->plen  = 0x04;  
  
  header->tpa = *tpa;  
  // obtain a copy of the hardware address...
  header->sha = *getHardwareAddress();
  // set the ip of the sender;
  header->spa = *ipGetAddress();
  
  if (tha == NULL)
  {
    header->operation  = ARP_OPCODE_REQUEST; 
    header->tha.octet1 = 0x0000;
    header->tha.octet2 = 0x0000;
    header->tha.octet3 = 0x0000;
    header->tha.octet4 = 0x0000;
    header->tha.octet5 = 0x0000;
    header->tha.octet6 = 0x0000;
    
    // ARP packets are broadcast packets... 
    struct macAddress_t mac
        = { .octet1 = 0xFF,  .octet2 = 0xFF,  .octet3 = 0xFF,
            .octet4 = 0xFF,  .octet5 = 0xFF,  .octet6 = 0xFF  };
    
    return onEthernetRequest(handle,ETHERTYPE_ARP, &mac);
  }
  else
  {
    header->operation = ARP_OPCODE_RESPONSE;
    header->tha = *tha;
    
    return onEthernetRequest(handle,ETHERTYPE_ARP, tha);
  }  
  
}

void onArpResponse(struct nicResponseHandle_t* response, struct macAddress_t* source)
{
  // drop if header invalid...
  if (nicResponseSize(response) < sizeof(struct soArpHeader_t))
    return;  
      
  struct soArpHeader_t header;
  nicResponseRead(response,(char*)(&header),sizeof(header));

  // check if the arprequest is understandable
  if ((header.htype != ntohs(0x0001)) || (header.ptype != ntohs(0x0800))
        || (header.hlen != 0x06) || (header.plen != 0x04))    
    return;
  
  // Someone talking to us?
  if (header.tpa != *ipGetAddress())
    return;
   
  switch (header.operation)
  {
    case ARP_OPCODE_REQUEST:
      // it's an arp discover, tell him who we are...
      arpSendResponse(&header.spa,&header.sha);
      // ... and then add the mac address...
      // ... yes, here is no break missing, its a fall through statement
    case ARP_OPCODE_RESPONSE:
      // it's an arp reply, update the arp table
      arpAddMacByIPv4(&header.spa,&header.sha);      
      break;
    default:
      break;
  }
  
  return;
}

uint8_t arpAddMacByIPv4(ipAddress_t* ip, struct macAddress_t* mac)
{
  int i = ARP_CACHE_SIZE;
  struct arpCacheEntry_t* oldest = NULL;
  
  while (i--)
  {
    // is it a direct hit?
    if (hArp.arpCache[i].ip == *ip)
    {
      hArp.arpCache[i].mac = *mac;
      hArp.arpCache[i].age = 255;
      return 0;
    }
    
    // we mark is entry as usable if it's overaged or was never used    
    if ((oldest == NULL) || (oldest->age < hArp.arpCache[i].age))
      oldest = &hArp.arpCache[i]; 
  }
  
  // overwrite the oldest entry
  oldest->ip = *ip;
  oldest->mac = *mac;
  oldest->age = 255;
  
  return 0;
}

uint8_t arpLookupMacByIPv4(ipAddress_t* ip, struct macAddress_t* mac)
{  

  int i = ARP_CACHE_SIZE;
  
  // check for a direct hit...
  while (i--)
  {
    if (hArp.arpCache[i].ip == *ip)
    {
      // copy the mac addres into the buffer
      *mac = hArp.arpCache[i].mac;
      // and reset the age, as is was most recently used
      hArp.arpCache[i].age = 255;
      return 0;
    }
  }
  
  // ... no hit? Send a request
  arpSendRequest(ip);
  // ...and return an error
  return 1;
}


void onArpTimeout()
{
  int i = ARP_CACHE_SIZE;
  while (i--)
  {
    if (hArp.arpCache[i].age == 0)
      continue;
    
    hArp.arpCache[i].age--;
  }
  
  // Every entry can age exactly 255 times, ...
  // ... as we age the arpcache very 6 seconds,...
  // ... this results in an total timeout of approx ...
  // ... 25 Minutes.
  timerAddTimeout(ARP_AGE_INTERVAL,onArpTimeout);
  
  return;
}

void onInitArp()
{
  int i = ARP_CACHE_SIZE;
  
  while (i--)
  {
    // a time to Live of 0 means empty
    hArp.arpCache[i].age = 0;
  }
  
  timerAddTimeout(ARP_AGE_INTERVAL,onArpTimeout);
  
  return;
}

