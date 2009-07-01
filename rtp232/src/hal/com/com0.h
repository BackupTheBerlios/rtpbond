#ifdef REQUIRES_SERIAL
  #ifndef COM0_H_
  #define COM0_H_

  #ifndef REQUIRES_QUEUE
    #error "com0.h depends on queue.h" 
  #endif
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!! DUE TO THE INTERRUPT ROUTINES, THIS HEADER CAN BE INCLUDED ONLY ONCE !!!
  // !!!                                                                      !!!
  // !!! THEREFOR DO NOT INCLUDE THIS FILE, AS IT IS ALREADY DONE BY serial.h !!!
  // !!!                                                                      !!!
  // !!! OTHERWISE THE LINKER WILL GENERATE AN ERROR DUE TO A DUPLICATE ISR   !!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 

  #include <stdint.h>
  #include <stdlib.h>

  #ifndef COM0_TIMEOUT
    // set timeout to 500ms
    //#define COM0_TIMEOUT 512	///alx
	#define COM0_TIMEOUT 512
  #endif

  #define COM0_MAX_RX_BUF 120	//alx =150
  #define COM0_MAX_TX_BUF 120

  //void com0Initialize(uint16_t baud, uint8_t settings);
  void com0Initialize(uint16_t prescaler, uint8_t parity, uint8_t stop_bits, uint8_t data_bits);
  void com0SendBytes(char* buffer, uint8_t length);
  void com0RecvBytes(char** buffer, uint8_t* length);

  void com0putBuffer(char *buffer,uint8_t len);
  void com0putChar(char data);
  uint8_t com0HasBytes();






  #endif /*COM0_H_*/
#endif
