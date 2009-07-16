#ifdef REQUIRES_SERIAL

  #ifndef SERIAL_H_
  #define SERIAL_H_
  
  #include <stdint.h>
  
  #ifndef SERIAL_EEPROM
    #warning "SERIAL Module: EEPROM base address not defined, using defaults"
    #define SERIAL_EEPROM 0x0300
  #endif

  #ifndef SERIAL_BAUD_PRESCALER
    #warning "SERIAL Module: Fallback Baud rate not defined, using defaults" 
    // Prescaler bzw. Baudrate 7 (115,2 kBaud)
    #define SERIAL_BAUD_PRESCALER 7
  #endif 
  
  #ifndef SERIAL_PARITY_MODE
    #warning "SERIAL Module: Fallback Parity mode not defined, using defaults" 
    // (keine Parität)
    #define SERIAL_PARITY_MODE 0	
  #endif  

  #ifndef SERIAL_STOPBITS
    #warning "SERIAL Module: Fallback Stopbits not defined, using defaults" 
    #define SERIAL_STOPBITS 0	
	//stop bits = 0 entspr 1
	//stop bits = 1 entspr 2
  #endif
 
  #ifndef DATA_BITS
    #warning "SERIAL Module: Fallback character size not defined, using defaults" 

	//data bits = 0 entspr 5
	//stop bits = 1 entspr 6
	//data bits = 2 entspr 7
	//stop bits = 3 entspr 8
	//stop bits = 7 entspr 9


    #define DATA_BITS 3
  #endif

  #ifndef SERIAL_BUFFER_SIZE_LIMIT
    #warning "SERIAL Module: Fallback BufferSizeLimit not defined, using defaults" 
    #define SERIAL_BUFFER_SIZE_LIMIT 120	
  #endif
  

  void onInitSerial();
  void onSerial();
  void serialSendBytes(char* buf, uint8_t len);
  
  extern uint8_t onSerialData(char* buf, uint8_t len);



  
  #endif /*SERIAL_H_*/
#endif
