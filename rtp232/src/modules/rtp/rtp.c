#ifdef REQUIRES_RTP
  
#include <string.h>

  #include "./rtp.h"
  #include "./../udp/udp.h"
  #include "./../ethernet/ethernet.h"
  #include "./../ip/ip.h"


  #include "./../../hal/com/com1_debug.h"
  #include "./../../modules/rcfg/rcfg.h"

 // #include
  #include "./../clock/clock.h"
  #include <stdlib.h>
  
  #include <avr/pgmspace.h>

  

  
  // This strucure is mainly used for casting....
  struct soRtpHeader_t
  {
    uint8_t version;
      // + version    : 2 
      // + padding    : 1
      // + extension  : 1
      // + CRSC Count : 4
    uint8_t type;
    uint16_t sequence;  //sequence number
    uint32_t timestamp;
    uint32_t ssrc;      //synchonizsation source identifier

  };

  struct cdpHeader_t
  {
    uint8_t length;
    uint8_t reserved;
    uint64_t timestamp;
  };
  
  
  uint16_t rtpSequence;



  
  void rtpSendCDPData(char* data, uint16_t len, 
      struct netAddress_t* destNetAddr, portAddress_t* srcPort)
  {

  
    	if(len!=0)
	{ 
  
  
  
    // create a new Packet, ...
    struct nicRequestHandle_t* handle = nicNewRequest();

 
	    // ... append the Payload
	    char* buf = nicAddPacketHeader(handle,150+sizeof(struct cdpHeader_t));
	    ((struct cdpHeader_t*)buf)->length = len;
	    ((struct cdpHeader_t*)buf)->reserved = 0;
	    ((struct cdpHeader_t*)buf)->timestamp = 0;
    
	    buf = buf+sizeof(struct cdpHeader_t);
    
	    while(len--)
	    {
	      buf[len] = data[len];
	    }
    
    onRtpRequest(handle,0x4D,destNetAddr,srcPort);
	}


	else //iam alive len=0
	{

	
    // create a new Packet, ...
    struct nicRequestHandle_t* handle = nicNewRequest();

 
	    // ... append the Payload
	    char* buf = nicAddPacketHeader(handle,150+sizeof(struct cdpHeader_t));
	    ((struct cdpHeader_t*)buf)->length = 0;
	    ((struct cdpHeader_t*)buf)->reserved = 0;
	    ((struct cdpHeader_t*)buf)->timestamp = 0;
    
	    buf = buf+sizeof(struct cdpHeader_t);
    
	    strcpy(buf,"iamalive\0");
    
    onRtpRequest(handle,0x4D,destNetAddr,srcPort);

	}



  
    return;
  }




 
  void rtpSendCommand(char* data, uint16_t len, 
      struct netAddress_t* destNetAddr, portAddress_t* srcPort)
  {
    // create a new Packet, ...
    struct nicRequestHandle_t* handle = nicNewRequest();
    


	if(len!=0)
	{
    // .... append the Payload...
	    char* buf = nicAddPacketHeader(handle,len+sizeof(struct cdpHeader_t));
	    ((struct cdpHeader_t*)buf)->length = len;
	    ((struct cdpHeader_t*)buf)->reserved = 0;
	    ((struct cdpHeader_t*)buf)->timestamp = 0;
    
	    buf = buf+sizeof(struct cdpHeader_t);
    
	    while(len--)
	    {
	      buf[len] = data[len];
	    }
	}
	else //iamalive paket
	{
		len=9;
		char* buf = nicAddPacketHeader(handle,len+sizeof(struct cdpHeader_t));
	    ((struct cdpHeader_t*)buf)->length = 0;	//here 0 because it is not a real command answer
	    ((struct cdpHeader_t*)buf)->reserved = 0;
	    ((struct cdpHeader_t*)buf)->timestamp = 0;
    
	    buf = buf+sizeof(struct cdpHeader_t);
    
	    while(len--)
	    {
	      buf[len] = data[len];
	    }

	}


  
    // ...and add the RTP Header...
    onRtpRequest(handle,0x25,destNetAddr,srcPort);
    
    return;
  }
  
  void onRtpRequest(struct nicRequestHandle_t* handle, uint8_t type,
      struct netAddress_t* destNetAddr, portAddress_t* srcPort)
  {
    struct soRtpHeader_t* header 
       = (struct soRtpHeader_t*)nicAddPacketHeader(handle,sizeof(struct soRtpHeader_t));
    
    header->version = 0x80;
    header->type = type;     
    header->timestamp = clockGetShortTimeStamp();
    // swap bytes by preprocessor macro...
    header->timestamp = htonl(header->timestamp);
    header->ssrc = 0x00;
    header->sequence = htons(rtpSequence);
    
    rtpSequence++;
    
    onUdpRequest(handle,destNetAddr,srcPort);  
    
    return;
  }
  
  void onRtpResponse(struct nicResponseHandle_t* response, struct netAddress_t* netAddr)
  {



    if (nicResponseSize(response) < sizeof(struct soRtpHeader_t))
      return;  
 
    struct soRtpHeader_t header;    
    nicResponseRead(response,(char*)(&header),sizeof(struct soRtpHeader_t));

    if (header.type == 0x4D)
    {
      struct cdpHeader_t cdpHeader;
      nicResponseRead(response,(char*)(&cdpHeader), sizeof(struct cdpHeader_t));
      
      if (cdpHeader.length == 0)
        return;
      
      char* buf = malloc(cdpHeader.length);
      uint8_t len = nicResponseRead(response,buf,cdpHeader.length);


	  
      
      onRtpData(buf,len);
      
      free(buf);
    }
#ifdef REQUIRES_RCFG
    else if (header.type == 0x25)
    {
      struct cdpHeader_t cdpHeader;
      nicResponseRead(response,(char*)(&cdpHeader), sizeof(struct cdpHeader_t));
      
      if (cdpHeader.length == 0)
        return;
      
      char* buf = malloc(cdpHeader.length);
      uint8_t len = nicResponseRead(response,buf,cdpHeader.length);

      onRtpControl(buf,len);

      free(buf);
    }  
#endif  
    return;
  }
  
  void onInitRtp()
  {
    return;
  }
  
#endif

