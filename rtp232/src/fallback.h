#ifndef FALLBACK_H_
#define FALLBACK_H_

#ifndef CONFIG_EEPROM
#warning "EEPROM config base address not defined, using defaults"
#define CONFIG_EEPROM 0x0400
#endif

#ifndef IP4_REMOTE_HOST_DATA
#warning "Fallback Remote Host for 'Data' not defined, using defaults"
#define IP4_REMOTE_HOST_DATA 0x0100A8C0 //192.168.0.1 !Reverse Byte order
#endif

#ifndef IP4_REMOTE_PORT_DATA
#warning "Fallback Remote Port for 'Data' not defined, using defaults"
#define IP4_REMOTE_PORT_DATA 7777
#endif

#ifndef IP4_REMOTE_HOST_CONFIG
#warning "Fallback Remote Host for 'Config' not defined, using defaults"
#define IP4_REMOTE_HOST_CONFIG 0x0100A8C0 //192.168.0.1 !Reverse Byte order
#endif

#ifndef IP4_REMOTE_PORT_CONFIG
#warning "Fallback Remote Port for 'Config' not defined, using defaults"
#define IP4_REMOTE_PORT_CONFIG 7777
#endif

#endif /*FALLBACK_H_*/

//#endif
