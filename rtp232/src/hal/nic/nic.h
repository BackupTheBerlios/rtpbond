#ifndef NIC_H_
#define NIC_H_

#include <stdint.h>
#include "./../../toolkit/netlib/netlib.h"
#include "./enc28j60.h"

#define NIC_PACKET_SEND_NOWAIT    (1<<0) // Does not wait until packet is send...
#define NIC_PACKET_SEND_NOERRCHK  (2<<0) // won't check for late collision ... 

struct nicRequestHandle_t
{
  struct soPacket_t* packet;
};

struct nicResponseHandle_t
{
  uint16_t length;
  uint16_t nextPacketPtr;
};

void nicInitialize(struct macAddress_t* mac);

// Function which handle incomming packets
struct nicResponseHandle_t* nicReceiveResponse();
void nicFreeResponse(struct nicResponseHandle_t* response);

uint16_t nicResponseSize(struct nicResponseHandle_t* response);
uint16_t nicResponseRead(struct nicResponseHandle_t* response, char* buffer, uint16_t len);


// Functions for outgoing packets
struct nicRequestHandle_t* nicNewRequest();
void nicFreeRequest(struct nicRequestHandle_t* handle);

uint16_t nicGetPacketSize(const struct nicRequestHandle_t* handle);
void* nicAddPacketHeader(struct nicRequestHandle_t* handle, uint16_t len);
uint8_t nicSendPacket(const struct nicRequestHandle_t* handle, uint8_t flags);




#endif /*NIC_H_*/
