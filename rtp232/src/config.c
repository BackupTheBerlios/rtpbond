#include <stdint.h>
#include <avr/eeprom.h>
#include "fallback.h"
#include "config.h"
#include "toolkit/netlib/netlib.h"

// global settings which are needed in more than one module
// e.g. Config.IPv4 is needed in "serialbridge" and "rcfg"
Config_t Config;

void onInitConfig()
{
  if ( !EepromReadConfig((void *)CONFIG_EEPROM, &Config.IPv4, sizeof(Config.IPv4)) ) {
    Config.IPv4.remoteAdrData.ip = IP4_REMOTE_HOST_DATA;
    Config.IPv4.remoteAdrData.port = IP4_REMOTE_PORT_DATA;
    Config.IPv4.remoteAdrConfig.ip = IP4_REMOTE_HOST_CONFIG;
    Config.IPv4.remoteAdrConfig.port = IP4_REMOTE_PORT_CONFIG;
    Config.IPv4.localSrcPort = 55042;
  }
}

#ifdef REQUIRES_RCFG
void configEepromSetConfig(ipAddress_t dIpData, portAddress_t dPortData,
			   ipAddress_t dIpConfig, portAddress_t dPortConfig,
			   portAddress_t sPort) {
  Config_t buf;
  if ( !EepromReadConfig((void*)CONFIG_EEPROM, &buf, sizeof(buf)) ) {
    // if eeprom data is invalid,
    // create new configuration based upon the fallback settings
    buf = Config;
  }

  // set new values
  if (dIpData != 0)
    buf.IPv4.remoteAdrData.ip = htonl(dIpData);
  if (dPortData != 0)
    buf.IPv4.remoteAdrData.port = dPortData;
  if (dIpConfig != 0)
    buf.IPv4.remoteAdrConfig.ip = htonl(dIpConfig);
  if (dPortConfig != 0)
    buf.IPv4.remoteAdrConfig.port = dPortConfig;
  if (sPort != 0)
    buf.IPv4.localSrcPort = sPort;

  EepromWriteConfig((void*)CONFIG_EEPROM, &buf, sizeof(buf));
  return;
}

void configEepromGetConfig(ipAddress_t * dIpData, portAddress_t * dPortData,
			   ipAddress_t * dIpConfig, portAddress_t * dPortConfig,
			   portAddress_t * sPort) {
  Config_t buf;
  if ( !EepromReadConfig((void*)CONFIG_EEPROM, &buf, sizeof(buf)) ) {
    // return fallback settings, if eeprom data is invalid
    buf = Config;
  }

  // assign values if pointer != NULL
  if (dIpData)
    *dIpData = ntohl(buf.IPv4.remoteAdrData.ip);
  if (dPortData)
    *dPortData = buf.IPv4.remoteAdrData.port;
  if (dIpConfig)
    *dIpConfig = ntohl(buf.IPv4.remoteAdrConfig.ip);
  if (dPortConfig)
    *dPortConfig = buf.IPv4.remoteAdrConfig.port;
  if (sPort)
    *sPort = buf.IPv4.localSrcPort;

  return;
}
#endif

#ifdef APPLICATION_BOOTLOADER

uint8_t EepromReadConfig(void * baseadr, void * buf, uint8_t len)
{
  // always use fallback config in bootloader application
  return 0;
}

#else

uint8_t EepromReadConfig(void * baseadr, void * buf, uint8_t len)
{
  struct netCheckSum_t checksum;

  // wait for eeprom
  while (eeprom_is_ready() == 0);

  // read config and checksum
  eeprom_read_block(buf,baseadr,len);
  eeprom_read_block(&checksum,baseadr+len,sizeof(checksum));

  // before calculating the checksum, we check if they are interlocked
  if (checksum.positive != ~checksum.negative)
    return 0; // read config failed (bad checksum)

  // as they are interlocked, it's time to proove the checksum
  if (calculateCheckSum(buf,len) != checksum.positive)
    return 0; // read config failed (bad checksum)

  return 1; // data is valid
}

void EepromWriteConfig(void * baseadr, void * buf, uint8_t len)
{
  struct netCheckSum_t checksum;
  eeprom_write_block(buf, baseadr, len);

  checksum.positive = calculateCheckSum(buf, len);
  checksum.negative = ~checksum.positive;
  eeprom_write_block(&checksum, baseadr+len, sizeof(checksum));
}

/*
void EepromUpdateChecksum()
{
  Config_t buf;
  struct netCheckSum_t checksum;
  if ( !EepromReadConfig(&buf, sizeof(Config_t)) ) {
    checksum.positive = calculateCheckSum(&buf, sizeof(Config_t));
    checksum.negative = ~checksum.positive;
    eeprom_write_block( (void*)CONFIG_EEPROM+sizeof(Config_t),
			&checksum, sizeof(struct netCheckSum_t) );
  }
}

// functions to read/write a single setting from/to eeprom

void EepromReadOption(void * reladr, void * buf, uint8_t len)
{
  void * adr = (void*)CONFIG_EEPROM + (reladr - (void*)&Config);
  eeprom_read_block(buf, adr, len);
  return;
}

void EepromWriteOption(void * reladr, void * buf, uint8_t len)
{
  void * adr = (void*)CONFIG_EEPROM + (reladr - (void*)&Config);
  eeprom_write_block(adr, buf, len);
  return;
}
*/

#endif
