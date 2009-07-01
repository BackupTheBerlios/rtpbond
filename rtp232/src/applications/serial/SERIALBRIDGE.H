#ifdef APPLICATION_SERIALBRIDGE
  #ifndef SERIALBRIDGE_H_
    #define SERIALBRIDGE_H_
  
    #ifndef REQUIRES_RTP
      #error "serialbridge.h requires RTP Kernel Module"
    #endif

    #ifndef REQUIRES_SERIAL
      #error "serialbridge.h requires Serial Kernel Module"
    #endif

    #include <stdint.h>
      
    void onRtpData(char* buf, uint8_t len);

    uint8_t onSerialData(char* buf, uint8_t len);
    
    void onInitApplication();

  #endif /*SERIALBRIDGE_H_*/
#endif
