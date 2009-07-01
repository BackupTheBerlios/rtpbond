#ifdef APPLICATION_BOOTLOADER

  #ifndef BOOTLOADER_H_
    #define BOOTLOADER_H_

    #ifndef REQUIRES_TFTP
      #error "bootloader.h requires TFTP Module"
    #endif

    #ifndef REQUIRES_BOOTLOADER
      #error "bootloader.h requires BOOTLOADER Module"
    #endif
    
    #include <stdint.h>

    uint8_t onTftpConnect(char* data, uint16_t len);
//void (onTftpData*)(uint16_t block, char* data, uint16_t len);
    void onTFtpComplete();
    void onTFtpTimeOut();
    
    void onInitApplication();

  #endif /*BOOTLOADER_H_*/
#endif


