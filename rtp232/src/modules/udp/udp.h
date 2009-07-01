#ifndef UDP_H_
#define UDP_H_

#include "./../../toolkit/netlib/netlib.h"
#include "./../../hal/nic/nic.h"
#include <stdint.h>



// This strucure is mainly used for casting....
struct soUdpHeader_t
{
  uint16_t srcPort;
  uint16_t destPort;
  uint16_t length;
  uint16_t checksum; // if not used set it to 0
};


void onUdpRequest(struct nicRequestHandle_t* handle,

struct netAddress_t* dest, portAddress_t* srcPort);

void onUdpResponse(struct nicResponseHandle_t* handle, ipAddress_t* ipAddress);

struct  soUdpHeader_t;

#endif /*UDP_H_*/
