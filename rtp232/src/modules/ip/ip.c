#include "./ip.h"
#include "./../udp/udp.h"
#include "./../arp/arp.h"
#include "../../config.h"
#include "./../../hal/com/com1_debug.h"
#include <stdlib.h>
#include <stdint.h>
#include <avr/eeprom.h>

#define IPTYPE_ICMP            1            
#define IPTYPE_TCP             6            
#define IPTYPE_UDP             17           




struct
{
  ipAddress_t host;        // 0xAA00A8C0 = 192.168.2.170
  ipAddress_t gateway;     // 0x0100A8C0 = 192.168.2.1
  ipAddress_t subnet;      // 0x00FFFFFF = 255.255.255.0
  
  uint16_t identification;   
} hIPv4 
    = {IP4_ADDRESS, IP4_GATEWAY, IP4_SUBNET, 0};

typedef struct {  
  ipAddress_t host;
  ipAddress_t subnet;
  ipAddress_t gateway;
} netIPv4Eeprom_t;

void onIPv4Response(struct nicResponseHandle_t* handle, struct macAddress_t* source)
{



  // if the packet length beyond expectation, drop it...
  if (nicResponseSize(handle) < sizeof(struct netIPv4Header_t))
    return;
  
  struct netIPv4Header_t header;
  
  nicResponseRead(handle,(char*)(&header),sizeof(struct netIPv4Header_t));
  
  // if the sender thinks, we have a different IP address...
  // ... we should definitivly drop the packet.
  if (header.destIP != *ipGetAddress())
    return;
    
  // if knows our IP we should add him to our Arp Cache...
  // ... but ignore all MAC Multi-/Boradcasts Packets...
  // ... otherwise we could poisen our Arp Cache.  
  if ( ! (source->octet1 & (1 << 0)))
  {
    // we should not lean the ip address if, its not within our subnet... 
    // ... instead we should update the arp entry for the gateway
    if ((header.srcIP & hIPv4.subnet) == ( hIPv4.gateway & hIPv4.subnet))
      arpAddMacByIPv4(&header.srcIP,source);
    else
      arpAddMacByIPv4(&hIPv4.gateway,source);
  }
     

     
  switch (header.protocol)
  {
    case 0x11:      
	
      onUdpResponse(handle, &header.srcIP);
      break;
  }
  
  return;
}

uint8_t onIPv4Request(struct nicRequestHandle_t* handle, ipAddress_t* destIP, uint8_t protocol)
{

  // we use lightweight arp, this means we do not wait for arp responses.
  // if it's a miss, we drop the packet...  
  struct macAddress_t mac;
  
  // check if destination address is within the subnet...
  // ... in case it is not, we have to send the packet to the gateway ...
  // ... yes this nested if is nasty, but we need the performance.
  if (arpLookupMacByIPv4(
      ((((*destIP) & hIPv4.subnet) == ( hIPv4.gateway & hIPv4.subnet))?destIP:&hIPv4.gateway),
      &mac) != 0)
  {
    nicFreeRequest(handle);
    return 1;    
  }
  
  // we passed the lookup, so it is save to assemble the IP header... 
  struct netIPv4Header_t* header
    = (struct netIPv4Header_t*) nicAddPacketHeader(handle,sizeof(struct netIPv4Header_t));
  
    // generate the IP Header...
  header->version        = ((0x4 << 4) & 0xF0 )| ( sizeof(struct netIPv4Header_t) >> 2 );
  header->typeOfService  = 0x00;
  header->totalLength    = htons(nicGetPacketSize(handle));
  header->identification = htons(hIPv4.identification);
  
  header->destIP = *destIP;
  header->srcIP  = *ipGetAddress();
  
  hIPv4.identification++;
  
  header->fragmentation  = htons(0x4000);
  header->timeToLive     = 128;
  header->protocol       = protocol;
 
  // when calculation the header checksum, it is assumed, that... 
  // ... the checksum itself is set to 0x00 
  header->headerCheckSum = 0x00;
  header->headerCheckSum 
      = calculateCheckSum(header,sizeof(struct netIPv4Header_t));
    
  return onEthernetRequest(handle,ETHERTYPE_IP4, &mac);
}

ipAddress_t* ipGetAddress()
{
  return &(hIPv4.host);
}

void onInitIPv4()
{
  // define a struct for the configuration...
  netIPv4Eeprom_t buf;

  // use eeprom settings if they are valid
  if ( EepromReadConfig((void*)IP4_EEPROM, &buf, sizeof(buf)) ) { 
    hIPv4.host    = buf.host;
    hIPv4.gateway = buf.gateway;
    hIPv4.subnet  = buf.subnet;
  }
  // else hIPv4 keeps the fallback settings
    
  return;
}

#ifdef REQUIRES_RCFG

void ipEepromSetConfig(ipAddress_t ip, ipAddress_t nm, ipAddress_t gw)
{
  netIPv4Eeprom_t buf;
  if ( !EepromReadConfig((void*)IP4_EEPROM, &buf, sizeof(buf)) )
  {
    // if eeprom data is invalid,
    // create new configuration based upon the fallback settings
    buf.host = hIPv4.host;
    buf.subnet = hIPv4.subnet;
    buf.gateway = hIPv4.gateway;
  }

  // set new values (network-byte-order!)
  if (ip != 0)
    buf.host = htonl(ip);

  if (nm != 0)
    buf.subnet = htonl(nm);

  if (gw != 0)
    buf.gateway = htonl(gw);

  EepromWriteConfig((void*)IP4_EEPROM, &buf, sizeof(buf));

  return;
}

void ipEepromGetConfig(ipAddress_t * ip, ipAddress_t * nm, ipAddress_t * gw)
{
  netIPv4Eeprom_t buf;
  if ( !EepromReadConfig((void*)IP4_EEPROM, &buf, sizeof(buf)) ) {
    // return fallback settings, if eeprom data is invalid
    buf.host = hIPv4.host;
    buf.subnet = hIPv4.subnet;
    buf.gateway = hIPv4.gateway;
  }

  // convert to host-byte-order and only assign values
  // if pointer != NULL
  if (ip)
  	*ip = ntohl(buf.host);
  if (nm)
    *nm = ntohl(buf.subnet);
  if (gw)
  	*gw = ntohl(buf.gateway);

  return;
}

#endif /* REQUIRES_RCFG */
