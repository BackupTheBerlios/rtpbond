#ifdef REQUIRES_SERIAL
#include "./serial.h"
#include "./../../hal/com/com0.h"
#include "./../../hal/com/com1_debug.h"
#include <stdlib.h>
#include "../../config.h"
#include "./../../modules/rcfg/rcfg.h"


extern struct StructCommandBuffer CommandBuffer;

struct
{
  uint16_t prescaler;
  uint8_t parity;
  uint8_t stop_bits;
  uint8_t data_bits;

} serialSettings = {SERIAL_BAUD_PRESCALER,SERIAL_PARITY_MODE,SERIAL_STOPBITS,DATA_BITS};
  
typedef struct {
  uint16_t prescaler;

  // size of struct has to be a multiple of 16 bits
  // this is a limitation of the checksum algorithm used
  uint8_t parity;
  uint8_t stop_bits;
  uint8_t data_bits;
  uint8_t unused;
} serialEeprom_t;




void onInitSerial()
{
  // define a struct for the configuration...
  serialEeprom_t buf;

  // use eeprom settings if they are valid
  if ( EepromReadConfig((void*)SERIAL_EEPROM, &buf, sizeof(buf)) ) {
    serialSettings.prescaler 	= buf.prescaler;
    serialSettings.parity  		= buf.parity;
	serialSettings.stop_bits 	= buf.stop_bits;
    serialSettings.data_bits  	= buf.data_bits;
  }
  // else serialSettings keeps the fallback settings

  // bitrate parity stop bits
  //com0Initialize(serialSettings.prescaler,serialSettings.settings);  //old one
  com0Initialize(serialSettings.prescaler,serialSettings.parity,serialSettings.stop_bits,serialSettings.data_bits);
  com1Initialize(0,0);
  return;
}

void serialSendBytes(char* buf, uint8_t len)
{


  //com0SendBytes(buf,len);
  com0putBuffer(buf, len);


}
  
void onSerial()
{ 


  if(CommandBuffer.NewCommand==1)
  {
  	CommandBuffer.NewCommand=0;
	onRtpControl(CommandBuffer.recvCommand, CommandBuffer.len);	
  }


  if (com0HasBytes() == 0)
    return;
       
  
  char* buf = NULL;
  uint8_t len = 0;
    
  com0RecvBytes(&buf,&len); 
  
   
  onSerialData(buf,len);
    
  free(buf);


  return;
}

#ifdef REQUIRES_RCFG

void serialEepromSetConfig(uint16_t prescaler, uint8_t parity, uint8_t stop_bits, uint8_t data_bits)
{
  serialEeprom_t buf;
  if ( !EepromReadConfig((void*)SERIAL_EEPROM, &buf, sizeof(buf)) ) {
    // if eeprom data is invalid,
    // create new configuration based on the fallback settings

	buf.prescaler 	= serialSettings.prescaler;
   	buf.parity  	= serialSettings.parity;
	buf.stop_bits 	= serialSettings.stop_bits;
    buf.data_bits  	= serialSettings.data_bits;
  }


	buf.prescaler 	= prescaler;
	buf.parity  	= parity;
	buf.stop_bits 	= stop_bits;
	buf.data_bits  	= data_bits;

  EepromWriteConfig((void*)SERIAL_EEPROM, &buf, sizeof(buf));

  return;
}

void serialEepromGetConfig(uint16_t * prescaler, uint8_t * parity, uint8_t * stop_bits, uint8_t * data_bits)
{
  serialEeprom_t buf;
  if ( !EepromReadConfig((void*)SERIAL_EEPROM, &buf, sizeof(buf)) ) {
    // return fallback settings, if eeprom data is invalid
    buf.prescaler 	= serialSettings.prescaler;
   	buf.parity  	= serialSettings.parity;
	buf.stop_bits 	= serialSettings.stop_bits;
    buf.data_bits  	= serialSettings.data_bits;
  }

  // BAUD = CLOCK_FREQUENCY / (16 * (UBBR + 1))
  //      = (CLOCK_FREQUENCY / 16) / (UBBR + 1)
  *prescaler=buf.prescaler;
  *parity=buf.parity;
  *stop_bits=buf.stop_bits;
  *data_bits=buf.data_bits;

}

#endif /* REQUIRES_RCFG */

#endif
