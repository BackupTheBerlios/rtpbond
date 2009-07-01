#ifndef IP_H_
  #define IP_H_

  #include "./../ethernet/ethernet.h"
  #include "./../../hal/nic/nic.h"

  #ifndef IP4_EEPROM
  //alx  #warning "IP Module: EEPROM base address not defined, using defaults"
    #define IP4_EEPROM 0x0100
  #endif

  #define IP4_EEPROM_LEN sizeof(ipAddress_t)*3

  #ifndef IP4_ADDRESS
    #error "IP Module: Fallback IPv4 Address not defined" 
  #endif

  #ifndef IP4_SUBNET
    #error "IP Module: Fallback Subnet not defined"
  #endif

  #ifndef IP4_GATEWAY
    #error "IP Module: Fallback Gateway not defined"
  #endif

  

  uint8_t onIPv4Request(struct nicRequestHandle_t* handle, ipAddress_t* destIP, uint8_t protocol);

  void onIPv4Response(struct nicResponseHandle_t* handle,  struct macAddress_t* source);

  void onInitIPv4();

  ipAddress_t* ipGetAddress();



struct netIPv4Header_t
{
  uint8_t version;
  // + headerlength  :4        // header length in multipes of 32 Bits, minimal 5*32Bit = 20 Byte
  // + version       :4        // IPv4 = 0x4  
  uint8_t typeOfService;     // Quality of Service
  uint16_t totalLength;      // IP Packet length, including the header 16bit = 65535 Bytes = 64kB
  uint16_t identification;   // unique id of the Datagramm
  uint16_t fragmentation;
  // + reserverd               :  1;
  // + unsigned dontFragemt    :  1; // indicates if Packet can be fragmented
  // + unsigned moreFragments  :  1; // indicates if more fragments are expected
  // + unsigned fragmendOffset : 13; //Position within the Fragment, offset in 64bit Steps
  uint8_t timeToLive;
  uint8_t protocol;               // UDP = 0x11: TCP = 0x06
  uint16_t headerCheckSum;        // Should be 11111111 (0xFF), ignored on most routers...
  ipAddress_t srcIP;       // Source IP Address in Big endian
  ipAddress_t destIP;  // Destination IP Address in  Big endian
  // Options in multipes of 32 bits, maximal 40 Bytes, zero padded.
  // until now not implemented...
};


#endif /*IP_H_*/
