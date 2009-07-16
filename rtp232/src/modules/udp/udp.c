#include "./udp.h"
#include "./../ip/ip.h"
#include "./../rtp/rtp.h"
#include "./../tftp/tftp.h"
#include "./../../toolkit/netlib/netlib.h"
#include "./../../hal/com/com1_debug.h"



void onUdpRequest(struct nicRequestHandle_t* handle,
    struct netAddress_t* dest, portAddress_t* srcPort)
{



  // ...generate the UDP Header...
  struct soUdpHeader_t* header 
     = (struct soUdpHeader_t*)nicAddPacketHeader(handle,sizeof(struct soUdpHeader_t));
  
  header->srcPort  = htons(*srcPort);  
  header->destPort = htons(dest->port);
  header->checksum = 0x00;
  header->length   = htons(nicGetPacketSize(handle));


  
  onIPv4Request(handle,&(dest->ip),0x11);
  return;
}

void onUdpResponse(struct nicResponseHandle_t* handle, ipAddress_t* ipAddress)
{
  // drop if header invalid...
  if (nicResponseSize(handle) < sizeof(struct soUdpHeader_t))
    return;  
  
  struct soUdpHeader_t header;  
  nicResponseRead(handle,(char*)(&header),sizeof(header));
  
  struct netAddress_t netAdr;
  netAdr.ip = *ipAddress;
  netAdr.port = ntohs(header.srcPort); 


  
  switch (ntohs(header.destPort))
  {
	

    #ifdef REQUIRES_RTP
    case RTP_SERVER_PORT  : 
      onRtpResponse(handle, &netAdr);
      break;
    #endif
    #ifdef REQUIRES_TFTP
    case TFTP_SERVER_PORT :
      onTftpResponse(handle, &netAdr);
      break;
    #endif
  } 
  return;
}

