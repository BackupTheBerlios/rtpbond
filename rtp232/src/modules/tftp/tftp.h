#ifdef REQUIRES_TFTP
  #ifndef TFTP_H_
  #define TFTP_H_

  #include <stdint.h>
  #include "./../../toolkit/netlib/netlib.h"
  #include "./../../hal/nic/nic.h"

  #ifndef TFTP_SERVER_PORT
    #define TFTP_SERVER_PORT 69
  #endif
  #ifndef TFTP_TIMEOUT
    // 60000 = 60sec, 6000 = 6sec
    #define TFTP_TIMEOUT 20000
  #endif

  void tftpSendAck(uint16_t block);
  void tftpSendError(uint16_t errCode, char* errMsg, uint8_t len);
  void tftpDisconnect();
  void onTftpResponse(struct nicResponseHandle_t* handle, struct netAddress_t* netAddr);

  extern uint8_t onTftpConnect(char* data, uint16_t len);
  extern void (*onTftpData)(uint16_t block, char* data, uint16_t len);
  extern void onTFtpComplete();
  extern void onTFtpTimeOut();
  
  #endif /*TFTP_H_*/
#endif
