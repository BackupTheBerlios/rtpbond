#ifdef APPLICATION_BOOTLOADER
  #include "./bootloader.h"
  #include "./../../modules/tftp/tftp.h"
  #include "./../../modules/arp/arp.h"
  #include "./../../modules/ip/ip.h"
  #include "./../../modules/timer/timer.h"
  #include <avr/boot.h>
  #include <avr/interrupt.h> 
  #include <avr/eeprom.h>
  //#include "./../modules/rtp/rtp.h"

  struct 
  {
    uint16_t block;
    uint32_t page; 
  } htftp;

  struct timerElement_t * tftpTimeout;

  void (*onTftpData)(uint16_t block, char* data, uint16_t len);
  void onTftpFlashData(uint16_t block, char* data, uint16_t len);
  void onTftpEepromData(uint16_t block, char * data, uint16_t len);  

  void onInitApplication()
  {
    // 3 gratious arps...
    arpSendRequest(ipGetAddress());
    arpSendRequest(ipGetAddress());
    arpSendRequest(ipGetAddress());
    
    // set 10 sec timeout...
    tftpTimeout = timerAddTimeout(10240, onTFtpComplete);
  }
  
  uint8_t compareStrings(char * s1, char * s2) {
    while ( *s1 && ( *s1++ == *s2++ ) );
    if ( *s1 == 0 && *s2 == 0 )
      return 1; // s1 == s2
    else
      return 0; // s1 != s2
  }

  // in case of an unsuccessful negotiation always return something not null!
  // otherwise the TFTP Socket will be jammed..
  // a non null value means a connection reject , 0 accepts the connection...
  uint8_t onTftpConnect(char* data, uint16_t len)
  {
    timerClearTimeout(tftpTimeout);
    //tftpTimeout = NULL;
    htftp.page = 0;
    htftp.block = 1;

    if (compareStrings(data, "firmware.bin")) {
      // set function-ptr to flash-processing function
      onTftpData = onTftpFlashData;
      //htftp.block = 1;
      return 0;
    }
    
    if (compareStrings(data, "eeprom.bin")) {
      // set function-ptr to eeprom-processing function
      onTftpData = onTftpEepromData;
      //htftp.block = 0;
      return 0;
    }

    // filename is neither firmware.bin nor eeprom.bin
    tftpSendError(0,"Filename has to be firmware.bin or eeprom.bin",53); 
    return 1;
   
    /* puts ist unabhaengig vom Controllertyp */
    /*void uart_puts (char *s)
    {
        while (*s)
        {   // so lange *s != '\0' also ungleich dem "String-Endezeichen" 
            uart_putc(*s);
            s++;
        }
    }*/    

  /*    
    htftp.block = 1;
    
    return 0;
  */
  }
  
  void write_flash_page(uint32_t page, uint8_t* buf)
  {
    uint16_t i;
    
    // Disable interrupts...
    cli();
    eeprom_busy_wait();
    boot_page_erase(page);
    boot_spm_busy_wait();
         
    for (i = 0; i< SPM_PAGESIZE; i+=2)
    {
      uint16_t w = *buf++;
      w += (*buf++) << 8;
      
      boot_page_fill(page+i,w);
    }
    
    boot_page_write(page);
    boot_spm_busy_wait();
    
    boot_rww_enable();
    
    // Enable interrupts...
    sei();
  }
  
  /*
  void ee_write(unsigned int addr, unsigned char data)
  {
    eeprom_busy_wait();
    eeprom_write_byte((unsigned char*)addr,data);
  }
  
  void flashBlock(char* data)
  {
    
  }*/
  

  
  void onTftpFlashData(uint16_t block, char* data, uint16_t len)
  {     
    // if (htftp.block != block)
    //  return;             
    // block muss kleiner als 480 da der ATMEGA max 480 Pages hat
    write_flash_page(htftp.page,(uint8_t*)data);
    htftp.page += SPM_PAGESIZE;
    write_flash_page(htftp.page,(uint8_t*)(data+SPM_PAGESIZE));
    htftp.page += SPM_PAGESIZE;
    
    tftpSendAck(block);
    htftp.block++;
    
    // eerprom
    /*eeprom_busy_wait();
    eeprom_write_byte(0x00,123456);*/

    /*struct netAddress_t adr;
    
    adr.ip =  0x0100A8C0; //192.168.0.1 !Reverse Byte order
    adr.port = 7777;
    portAddress_t port = 55042;
    rtpSendData(data,len,&adr,&port);*/    
    
    return;
  }

  void onTftpEepromData(uint16_t block, char* data, uint16_t len) {
    //if (block == htftp.block) {
      // Disable interrupts...
      cli();

      eeprom_write_block(data, (void*)htftp.page, len);

      // Enable interrupts...
      sei();

      htftp.page += len;
      htftp.block++;
    //}

    tftpSendAck(block);

    return;
  }

  
  // This function is invoked, as soon a tftp connection is terminated.
  // Refused connects, completed transfers or timeouts are causes for 
  // a disconnect. After the disconnect it is not possible to send any data.  
  void onTFtpComplete()
  {
    // disable interrupts...
    cli();
    /* Enable change of interrupt vectors */
    MCUCR = (1<<IVCE);
    /* restore interrupts vectors */
    MCUCR = (0<<IVSEL);
    
    RAMPZ &= ~(1 << RAMPZ0);
      
    asm("jmp 0"); 

    /*
      we'll never reach this part
    ((void(*) ())0x0000)();  
          
    return;
	*/
  }
  
  void onTFtpTimeOut()
  {
    /*struct netAddress_t adr;
  
    adr.ip =  0x0100A8C0; //192.168.0.1 !Reverse Byte order
    adr.port = 7777;
    portAddress_t port = 55042;
    rtpSendData("Timeout",7,&adr,&port);*/
    return;
  }
#endif
