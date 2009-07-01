#ifdef REQUIRES_RCFG

#include "./parser.h"

/*
uint8_t rcfg_parseUInt8(char ** pbuf)
{
  char * buf = *pbuf;

  uint8_t value = 0;
  while ( (*buf) >= '0' && (*buf) <= '9' )
  {
    value *= 10;
    value += (*buf) - '0';
    buf++;
  }

  *pbuf = buf;
  return value;
}

uint16_t rcfg_parseUInt16(char ** pbuf)
{
  char * buf = *pbuf;

  uint16_t value = 0;
  while ( (*buf) >= '0' && (*buf) <= '9' )
  {
    value *= 10;
    value += (*buf) - '0';
    buf++;
  }

  *pbuf = buf;
  return value;
}
*/

uint32_t rcfg_parseUInt(char ** pbuf)
{
  char * buf = *pbuf;

  uint32_t value = 0;
  while ( (*buf) >= '0' && (*buf) <= '9' )
  {
    value *= 10;
    value += (*buf) - '0';
    buf++;
  }

  *pbuf = buf;
  return value;
}

/*
uint8_t rcfg_parseHex8(char ** pbuf)
// expects exactly 2 chars!
{
  char * buf = *pbuf;

  uint8_t value = 0;
  if ( (*buf) >= '0' && (*buf) <= '9' )
    value = (*buf) - '0';
  else if ( (*buf) >= 'a' && (*buf) <= 'f' )
    value = (*buf) - 'a' + 10;
  else if ( (*buf) >= 'A' && (*buf) <= 'F' )
    value = (*buf) - 'A' + 10;

  value <<= 4;
  buf++;

  if ( (*buf) >= '0' && (*buf) <= '9' )
    value += (*buf) - '0';
  else if ( (*buf) >= 'a' && (*buf) <= 'f' )
    value += (*buf) - 'a' + 10;
  else if ( (*buf) >= 'A' && (*buf) <= 'F' )
    value += (*buf) - 'A' + 10;

  buf++;

  *pbuf = buf;
  return value;
}
*/

uint32_t rcfg_parseUHex(char ** pbuf)
// expects exactly 2 chars!
{
  char * buf = *pbuf;

  uint8_t nibble = 0;
  uint32_t value = 0;

  while (1/*nibble <= 0x0f*/) {
    value <<= 4;
	value += nibble;
  
    if ( (*buf) >= '0' && (*buf) <= '9' )
      nibble = (*buf) - '0';
    else if ( (*buf) >= 'a' && (*buf) <= 'f' )
      nibble = (*buf) - 'a' + 10;
    else if ( (*buf) >= 'A' && (*buf) <= 'F' )
      nibble = (*buf) - 'A' + 10;
    else {
	  break;
	  /*
      nibble = 0xff;
	  buf--;
	  */
    }

    buf++;
  }

  *pbuf = buf;
  return value;
}

ipAddress_t rcfg_parseIP(char ** pbuf)
{
  char * buf = *pbuf;

  ipAddress_t ip = 0;
  ip = (ipAddress_t)rcfg_parseUInt(&buf) << 24;
  if (*buf != '.')
    return 0;
  buf++;
  ip |= (ipAddress_t)rcfg_parseUInt(&buf) << 16;
  if (*buf != '.')
    return 0;
  buf++;
  ip |= (ipAddress_t)rcfg_parseUInt(&buf) << 8;
  if (*buf != '.')
    return 0;
  buf++;
  ip |= (ipAddress_t)rcfg_parseUInt(&buf); 

  *pbuf = buf;
  return ip;
}

#endif
