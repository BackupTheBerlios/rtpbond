#ifdef REQUIRES_SERIAL
  #ifndef COM1_H_
  #define COM1_H_


  #include <stdint.h>
  #include <stdlib.h>



struct StructCommandBuffer
{
	// define our buffer...
	 char recvCommand[40];
	// ... and a helper, which refers to the next empty char
	 uint8_t NewCommand;     
	 uint8_t len;
};



  //for testing com1
  void com1Initialize(uint16_t baud, uint8_t settings);
  char getChar();
  void putChar(char data);
  void putString(char *buffer);
void ExecuteCommand(char* command,uint8_t len);



  #endif /*COM1_H_*/
#endif
