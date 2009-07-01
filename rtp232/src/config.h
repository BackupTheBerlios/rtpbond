#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include "toolkit/netlib/netlib.h"

typedef struct {
  struct netAddress_t remoteAdrData;
  struct netAddress_t remoteAdrConfig;
  portAddress_t localSrcPort;
} IPv4Config_t;

typedef struct {
  IPv4Config_t IPv4;
} Config_t;

void onInitConfig();
uint8_t EepromReadConfig(void * baseadr, void * buf, uint8_t len);
void EepromWriteConfig(void * baseadr, void * buf, uint8_t len);

//void ConfigEepromReadOption(void * reladr, void * buf, uint8_t len);
//void EepromWriteOption(void * reladr, void * buf, uint8_t len);

extern Config_t Config;

#endif /*CONFIG_H_*/
