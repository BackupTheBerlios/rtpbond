#ifndef RCFG_COMMANDS_H_
#define RCFG_COMMANDS_H_

#include <stdint.h>

#define RCFG_SUCCESS 0
#define RCFG_FAIL 1

uint8_t rcfg_getConfig(char * buf);

uint8_t rcfg_setMAC(char * buf);
uint8_t rcfg_getMAC(char * buf);

uint8_t rcfg_getIP(char * buf);
uint8_t rcfg_setIP(char * buf);
uint8_t rcfg_getNM(char * buf);
uint8_t rcfg_setNM(char * buf);
uint8_t rcfg_getGW(char * buf);
uint8_t rcfg_setGW(char * buf);
uint8_t rcfg_getNet(char * buf);
uint8_t rcfg_setNet(char * buf);
// remote address for data
uint8_t rcfg_getDIP_Data(char *buf);
uint8_t rcfg_setDIP_Data(char *buf);
uint8_t rcfg_getDPORT_Data(char *buf);
uint8_t rcfg_setDPORT_Data(char *buf);
// remote address for config
uint8_t rcfg_getDIP_Config(char *buf);
uint8_t rcfg_setDIP_Config(char *buf);
uint8_t rcfg_getDPORT_Config(char *buf);
uint8_t rcfg_setDPORT_Config(char *buf);
// com 0 settings
uint8_t rcfg_setBaud(char * buf);
uint8_t rcfg_getBaud(char * buf);
uint8_t rcfg_setParity(char * buf);
uint8_t rcfg_getParity(char * buf);
uint8_t rcfg_setStopBits(char * buf);
uint8_t rcfg_getStopBits(char * buf);
uint8_t rcfg_setDataBits(char * buf);
uint8_t rcfg_getDataBits(char * buf);

uint8_t rcfg_Reboot(char * buf);
uint8_t rcfg_Disable(char * buf);

uint8_t Config_is_Disabled();

#endif
