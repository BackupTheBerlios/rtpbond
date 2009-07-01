#ifndef NETLIB_H_
#define NETLIB_H_
  
  #include <stdint.h>
  
  typedef uint32_t ipAddress_t;
  typedef uint16_t portAddress_t;
    
  struct netAddress_t 
  {
    ipAddress_t ip;
    portAddress_t port;
  };

  struct netCheckSum_t
  {
    uint16_t positive;
    uint16_t negative;
  };
  
//  #define MAC(A,B,C,D,E,F) {A, B, C, D, E, F}
//  #define IP(A) ()
  //typedef uint8_t mac_t[6];
  
  #define highWord(a) (( (a) & 0xFF00) >> 8)
  #define lowWord(a)  ( (a) & 0x00FF)
  
  struct macAddress_t
  {
    uint8_t octet1;
    uint8_t octet2;
    uint8_t octet3;
    uint8_t octet4;
    uint8_t octet5;
    uint8_t octet6; 
  };  

  
  /*
   * htons(): reorder the bytes of a 16-bit unsigned value from processor order to network order. The macro name can be read "host to network short."
   * htonl(): reorder the bytes of a 32-bit unsigned value from processor order to network order. The macro name can be read "host to network long."
   * ntohs(): reorder the bytes of a 16-bit unsigned value from network order to processor order. The macro name can be read "network to host short."
   * ntohl(): reorder the bytes of a 32-bit unsigned value from network order to processor order. The macro name can be read "network to host long."
   */

     /*((((uint16_t)(A) & 0xFF00) >> 8) | \
                     (((uint16_t)(A) & 0x00FF) << 8))
  */
  #define ntohs(A)   htons(A)
  #define ntohl(A)   htonl(A)

  #define htonl(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
                     (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                     (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                     (((uint32_t)(A) & 0x000000ff) << 24))
  
  #define htons(A) ((((uint16_t)(A) & 0xFF00) >> 8) | \
      (((uint16_t)(A) & 0x00FF) << 8))

  #define ntohs(A)   htons(A)
  /*uint16_t htons(uint16_t buffer) 
  {
    return ((buffer & 0xFF00) >> 8 ) | ((buffer & 0x00FF) << 8);
  };


  uint16_t ntohs(uint16_t buffer) 
  {
    return ((buffer & 0xFF00) >> 8 ) | ((buffer & 0x00FF) << 8);
  };*/  
 
  uint16_t calculateCheckSum(void* bytes, uint16_t len);  
#endif /*NETLIB_H_*/
