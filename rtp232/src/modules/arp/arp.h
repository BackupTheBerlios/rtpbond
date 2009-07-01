#ifndef ARP_H_
#define ARP_H_

  #include "./../../toolkit/netlib/netlib.h"
  #include "./../ethernet/ethernet.h"
  #include "./../../hal/nic/nic.h"

  #ifndef ARP_CACHE_SIZE
    #define ARP_CACHE_SIZE 5
  #endif

  #ifndef ARP_AGE_INTERVAL
    #define ARP_AGE_INTERVAL 6000
  #endif

  uint8_t onArpRequest(struct nicRequestHandle_t* handle, ipAddress_t* destIP, struct macAddress_t* destMac);

  void afterArpRequest(struct nicRequestHandle_t* handle);

  void onArpRequestError(struct nicRequestHandle_t* handle);

  void onArpResponse(struct nicResponseHandle_t* handle, struct macAddress_t* source);

  uint8_t arpLookupMacByIPv4(ipAddress_t* ip, struct macAddress_t* mac);
  uint8_t arpAddMacByIPv4(ipAddress_t* ip, struct macAddress_t* mac);
  uint8_t arpSendRequest(ipAddress_t* tpa);
  
  void onInitArp();

#endif /*ARP_H_*/
