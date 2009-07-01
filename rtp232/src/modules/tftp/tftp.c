#ifdef REQUIRES_TFTP 

  #include "./tftp.h"
  #include "./../udp/udp.h" 
  #include "./../timer/timer.h"    
  #include <stdlib.h>

  // Local handle for interprocess commuincation...
  struct 
  {
    struct netAddress_t* destNetAddr;
    struct timerElement_t* timer;
  } hTftp = { NULL, NULL };

  void onTimeoutTftp()
  {
    // as first action we have to null the timer element...
    // ...otherwise we would doublefree memory
    hTftp.timer = NULL;
    tftpDisconnect();
    onTFtpTimeOut();
  }
  
  
  void tftpSendAck(uint16_t block)
  {
    if (hTftp.destNetAddr == NULL)
      return;
    
    struct nicRequestHandle_t* handle = nicNewRequest();
    
    char* buf = nicAddPacketHeader(handle,4);
   
    *((uint16_t*)buf) = 0x0400;
    *((uint16_t*)(buf+2)) = block;
    
    portAddress_t port = 0x0045;    
    onUdpRequest(handle,hTftp.destNetAddr,&port);
    // no need to free the packet, this happens automagically
    
    timerClearTimeout(hTftp.timer);
    hTftp.timer = timerAddTimeout(TFTP_TIMEOUT,onTimeoutTftp);
  }
  
  void tftpSendError(uint16_t errCode, char* errMsg, uint8_t len)
  {
    if (hTftp.destNetAddr == NULL)
      return;
    
    struct nicRequestHandle_t* handle = nicNewRequest();
    
    char* buf = nicAddPacketHeader(handle,4+len+1);
    
    *((uint16_t*)buf) = 5;
    *((uint16_t*)(buf+2)) = errCode;
    
    (buf+4)[len+1] = '\0';    
    while(len--)
    {
      (buf+4)[len] = errMsg[len];
    }
         
    uint16_t port = 69;
    onUdpRequest(handle,hTftp.destNetAddr,&port);
    // Packets are automagically freed upon successfull send..
    
    timerClearTimeout(hTftp.timer);
    hTftp.timer = timerAddTimeout(TFTP_TIMEOUT,onTimeoutTftp);
  }

  void tftpConnect(struct netAddress_t* netAddr)
  {
    hTftp.destNetAddr = malloc(sizeof (struct netAddress_t));
    *(hTftp.destNetAddr) = *netAddr;
    hTftp.timer = NULL;

	//alx free?
  }

  void tftpDisconnect()
  {
    if (hTftp.timer != NULL)
     timerClearTimeout(hTftp.timer);
    
    free(hTftp.destNetAddr);
    hTftp.destNetAddr = NULL;
    hTftp.timer = NULL;    
  }

  void onTftpResponse(struct nicResponseHandle_t* response, struct netAddress_t* netAddr)
  {    
    // This TFTP Server supports only write request (WRQ)...    
    // ... additionally we don't accept concurrent connection
    
    // Read TFTP Payload into Memory...
    uint16_t len = nicResponseSize(response);
    char* buf = malloc(len);
    // ... due to the huge payload size, malloc might fail...
    if (buf == NULL)
      return;
    
    // We got the memory allocated, so let use it and load ...
    // ... the Payload into our buffer memory.
    nicResponseRead(response,buf,len);
    
    // parse the opcode of the header
    switch (*((uint16_t*)buf))
    {
      case 0x0200: // it is a Write ReQuest (WRQ)
        // avoid concurrent incomming requests...
        if (hTftp.destNetAddr != NULL)
          break;
        
        // accept the connection...
        tftpConnect(netAddr);
            
        // connection successfully nagociated?
        if (onTftpConnect((buf+2),len-2) != 0)
        {
          tftpDisconnect();
          break;
        }
        // accept the connection...
        tftpSendAck(0x00);
        
        // as the writerequest is accepted...
        // ... ensure that the tftp socket resets itself...
        // ... on 60 seconds inactivity
        hTftp.timer = timerAddTimeout(TFTP_TIMEOUT, onTimeoutTftp);
        break;
        
      case 0x0300: // we received Data...
        
        // are we expecting data?
        if (hTftp.destNetAddr == 0)
          break;
    
        if ((hTftp.destNetAddr->ip != netAddr->ip)
            || (hTftp.destNetAddr->port != netAddr->port))          
          break;
        
        onTftpData(*((uint16_t*)(buf+2)), (buf+4),len-4);     
        
        // disconnect, if we received the last packet
        if ((len-4) <= 511)
        {
          tftpDisconnect();
          onTFtpComplete();
        }
        else
        {        
          // update the timeout... 60 more seconds
          timerClearTimeout(hTftp.timer);
          hTftp.timer = timerAddTimeout(TFTP_TIMEOUT,onTimeoutTftp);
        }
        break;
      default:
        // we received an unknown or unimplented opcode...
        // ... we ignore it according to the RFC.
        break;
    }

    // ...finally free the buffer
    free(buf);
    
    return;
  }
  
#endif
