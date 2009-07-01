#ifdef REQUIRES_RTP
  #ifndef RTP_H_
  #define RTP_H_
  
  #include "./../../hal/nic/nic.h"
  #include "./../../toolkit/netlib/netlib.h"
  #include <stdint.h>
  
  #define RTP_SERVER_PORT 7777
  #define RTP_CLIENT_PORT 55042
  
  
  void rtpSendCDPData(char* data, uint16_t len, 
      struct netAddress_t* destNetAddr, portAddress_t* srcPort);
  
  void rtpSendCommand(char* data, uint16_t len, 
      struct netAddress_t* destNetAddr, portAddress_t* srcPort);
  
  void onRtpRequest(struct nicRequestHandle_t* handle, uint8_t type,
      struct netAddress_t* destNetAddr, portAddress_t* srcPort);
  
  void onRtpResponse(struct nicResponseHandle_t* response, struct netAddress_t* netAddr);
  
  void onInitRtp();
  

  
  #endif /*RTP_H_*/
#endif

