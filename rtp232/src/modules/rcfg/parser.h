#ifndef RCFG_PARSER_H_
#define RCFG_PARSER_H_

#include "../../toolkit/netlib/netlib.h"

uint32_t rcfg_parseUInt(char ** pbuf);
uint32_t rcfg_parseUHex(char ** pbuf);
ipAddress_t rcfg_parseIP(char ** pbuf);

#endif /* RCFG_PARSER_H_ */
