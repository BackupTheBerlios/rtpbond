#include "./netlib.h"

  // Kann nur die checksummer über ein e gerade anzahl berechen... 
  uint16_t calculateCheckSum(void* bytes, uint16_t len)
  {
   // Amazingly the C code is translated in optimal assembler code...
   register uint32_t checksum = 0x00000000;
   
   // as we are processing full words, the length of the data ...
   // ... has to be odd. If it's even, we would loose the last byte 
   // ... so we have to take some precautions
   if (len & 0x00000001)
     checksum+= (((uint8_t*)bytes)[len]);
   
   // now, as we ensured an odd length, we can convert the length...
   // ... from bytes to words  
   len = len >> 1;

   // calculate the checksum, wordwise
   while (len--)
     checksum += (((uint16_t *)bytes)[len]);

   // add overflows to the checksum 
   while (checksum>>16)
     checksum = (checksum & 0xffff) + (checksum >> 16);

   return ~((uint16_t)checksum);
 };

