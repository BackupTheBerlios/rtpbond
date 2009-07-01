#ifndef ETHERNET_H_
  #define ETHERNET_H_

  #include <stdint.h>
  #include "./../../toolkit/netlib/netlib.h"
  #include "./../../hal/nic/nic.h"

  // Define Ethertypes in network byte order!
  #define ETHERTYPE_IP4 0x0008
  #define ETHERTYPE_IP6 0xDD86
  #define ETHERTYPE_ARP 0x0608

  #ifndef MAC_EEPROM
 //alx   #warning "MAC Module: EEPROM base address not defined, using defaults"
    #define MAC_EEPROM 0x0200
  #endif

  #ifndef MAC_GROUP1
    #error "Ethernet Module: First word of fallback MAC Address not specified"
  #endif

  #ifndef MAC_GROUP2
    #error "Ethernet Module: Second word of fallback MAC Address not specified"
  #endif

  #ifndef MAC_GROUP3
    #error "Ethernet Module: Third word of fallback MAC Address not specified"
  #endif



  struct macAddress_t* getHardwareAddress();

  uint8_t onEthernetRequest(struct nicRequestHandle_t* handle, uint16_t type, 
                        struct macAddress_t* destMac);

  void onEthernet();

  void onInitEthernet();



  struct netEthernetHeader_t
{
  struct macAddress_t destination; // 6bytes 
  struct macAddress_t source;     // 6bytes
  uint16_t  etherType;         // 2bytes
};

#endif /*ETHERNET_H_*/
