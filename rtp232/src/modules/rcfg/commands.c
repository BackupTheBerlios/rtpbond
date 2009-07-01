#ifdef REQUIRES_RCFG

#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "../rtp/rtp.h"
#include "../../config.h"
#include "commands.h"
#include "parser.h"

#include "./../../hal/com/com1_debug.h"
#include "./../ip/ip.h"
#include "./../serial/serial.h"

// FIXME: check for command completeness ( -> ?$)
// FIXME: check return values for success/failure

extern void ethernetEepromSetConfig(uint8_t * mac);
extern void ethernetEepromGetConfig(uint8_t * mac);

extern void ipEepromSetConfig(ipAddress_t ip, ipAddress_t nm, ipAddress_t gw);
extern void ipEepromGetConfig(ipAddress_t * ip, ipAddress_t * nm, ipAddress_t * gw);

extern void configEepromSetConfig(ipAddress_t dIpData, portAddress_t dPortData, ipAddress_t dIpConfig, portAddress_t dPortConfig, portAddress_t sPort);
extern void configEepromGetConfig(ipAddress_t * dIpData, portAddress_t * dPortData, ipAddress_t * dIpConfig, portAddress_t * dPortConfig, portAddress_t * sPort);

extern void serialEepromSetConfig(uint16_t prescaler, uint8_t parity, uint8_t stop_bits, uint8_t data_bits);
extern void serialEepromGetConfig(uint16_t * prescaler, uint8_t * parity, uint8_t * stop_bits, uint8_t * data_bits);

static uint8_t isDisabled = 0;

uint8_t stringLength(char * buf)
{
  uint8_t len = 0;
  while (*buf++)
    len++;
  return len;
}

char * ip2str(char * buf, ipAddress_t ip)
{
  union {
    ipAddress_t ip;
    uint8_t byte[3];
  } conv;
  conv.ip = ip;

  int8_t i;
  for (i=3; i>=0; i--) {
    utoa(conv.byte[i], buf, 10);
    while( *buf >= '0' && *buf <= '9')
      buf++;
    *buf++ = '.';
  }
  *(--buf) = '\0';

  // return current position in buf
  return buf;
}



uint8_t rcfg_getConfig(char * buf)
{


//Address 100 IP4_EEPROM

	ipAddress_t ip, nm, gw;
	ipEepromGetConfig(&ip, &nm, &gw);

	char sendbuf[50] = {0}; // clear buffer;
	char * newpos;
	newpos = ip2str(sendbuf, ip);
	*newpos++ = ' ';
	newpos = ip2str(newpos, nm);
	*newpos++ = ' ';
	ip2str(newpos,gw);

	rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

	putString("\r\nConfig-Overview: ");
	putString("\r\nAb Adresse 0x100: ");



	putString("(IP,Netmask,GW) ");
	putString(sendbuf);


	//Address 200 MAC_EEPROM
	

	uint8_t mac[6];
	ethernetEepromGetConfig(mac);

	sendbuf[0] = '\0'; // clear buffer
	char * bufpos = sendbuf;
	uint8_t i;
	for (i=0; i<=5; i++) {
	utoa(mac[i], bufpos, 16);
	while ( (*bufpos >= '0' && *bufpos <= '9') ||
	    (*bufpos >= 'a' && *bufpos <= 'f') ||
	    (*bufpos >= 'A' && *bufpos <= 'F') )
	  bufpos++;
	*bufpos++ = ':';
	}
	*(--bufpos) = '\0';

	putString("\r\nAb Adresse 0x200: ");
	putString("(MAC-Address) ");
	putString(sendbuf);




	//Address 300 SERIAL_EEPROM

	uint16_t prescaler=0;
	uint8_t parity=0;
	uint8_t stop_bits=0;
	uint8_t data_bits=0;

	serialEepromGetConfig(&prescaler,&parity,&stop_bits,&data_bits);
	stop_bits+=1;
	data_bits+=5;

	putString("\r\nAb Adresse 0x300: ");


	putString("(Prescaler,Parity,StopBits,DataBits)  ");

	char buffer[10];

	ultoa(prescaler,buffer,10);
	putString(buffer);

	putString("  ");

	ultoa(parity,buffer,10);
	putString(buffer);

	putString("  ");

	ultoa(stop_bits,buffer,10);
	putString(buffer);

	putString("  ");

	ultoa(data_bits,buffer,10);
	putString(buffer);

	putString("\r\nPrescaler belongs to this baudrate: ");


	uint32_t baud=F_CPU  / (16 * (prescaler + 1));
	ultoa(baud, buffer, 10);
	putString(buffer);

	putString("\r\nParity Value means: ");
	switch (parity)
	{
		case 0:
			putString("Parity Disabled: ");
		break;
		case 1:
			putString("Parity Reserved: ");
		break;
		case 2:
			putString("Parity Enabled, Even Parity: ");
		break;
		case 3:
			putString("Parity Enabled, Odd Parity: ");
		break;
	}



	//Address 400 CONFIG_EEPROM


	//data
	ipAddress_t dip_data;
	ipAddress_t dip_config;
	portAddress_t dport_data;
	portAddress_t dport_config;
	portAddress_t  sPort;


	configEepromGetConfig(&dip_data, &dport_data, &dip_config, &dport_config, &sPort);

	



	putString("\r\nAb Adresse 0x400: ");
	putString("(IP_d,port_d,IP_c,port_c,sPort) ");

	putString("\r\n");
	ip2str(sendbuf, dip_data);
	putString(sendbuf);
	putString(" ");

	ultoa(dport_data,sendbuf, 10);
	putString(sendbuf);
	putString(" ");

	ip2str(sendbuf, dip_config);
	putString(sendbuf);
	putString(" ");

	ultoa(dport_config,sendbuf,10);
	putString(sendbuf);
	putString(" ");

	ultoa(sPort,sendbuf, 10);
	putString(sendbuf);
	putString("\r\n");








	return RCFG_SUCCESS;
}



uint8_t rcfg_setMAC(char * buf)
{
  uint8_t mac[6];
  uint8_t i;
  for (i=0; i<=5; i++) {
    mac[i] = rcfg_parseUHex(&buf);
    buf++; // skip byte delimiter (usually ':')
  }
  ethernetEepromSetConfig(mac);

  return RCFG_SUCCESS;
}

uint8_t rcfg_getMAC(char * buf)
{
  uint8_t mac[6];


  char sendbuf[17] = {0}; // clear buffer
  char * bufpos = sendbuf;
  uint8_t i;
  for (i=0; i<=5; i++) {
    utoa(mac[i], bufpos, 16);
    while ( (*bufpos >= '0' && *bufpos <= '9') ||
	    (*bufpos >= 'a' && *bufpos <= 'f') ||
	    (*bufpos >= 'A' && *bufpos <= 'F') )
	  bufpos++;
    *bufpos++ = ':';
  }
  *(--bufpos) = '\0';

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nMAC-Address: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_setIP(char * buf)
{
  ipAddress_t ip = rcfg_parseIP(&buf);

  ipEepromSetConfig(ip,0,0);

  return RCFG_SUCCESS;
}

uint8_t rcfg_setNM(char * buf)
{
  ipAddress_t nm = rcfg_parseIP(&buf);

  ipEepromSetConfig(0,nm,0);

  return RCFG_SUCCESS;
}

uint8_t rcfg_setGW(char * buf)
{
  ipAddress_t gw = rcfg_parseIP(&buf);

  ipEepromSetConfig(0,0,gw);

  return RCFG_SUCCESS;
}

uint8_t rcfg_setNet(char * buf)
{
  ipAddress_t ip = rcfg_parseIP(&buf);
  while(*buf == ' ') // skip whitespace(s)
    buf++;
  ipAddress_t nm = rcfg_parseIP(&buf);
  while(*buf == ' ')
    buf++;
  ipAddress_t gw = rcfg_parseIP(&buf);

  ipEepromSetConfig(ip,nm,gw);

  return RCFG_SUCCESS;
}

uint8_t rcfg_getIP(char * buf)
{
  ipAddress_t ip;
  ipEepromGetConfig(&ip, NULL, NULL);

  char sendbuf[15] = {0}; // clear buffer
  ip2str(sendbuf, ip);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nIP-Address: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_getNM(char * buf)
{
  ipAddress_t nm;
  ipEepromGetConfig(NULL, &nm, NULL);

  char sendbuf[15] = {0}; // clear buffer
  ip2str(sendbuf, nm);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);


  putString("\r\nNetwork-Mask: ");
  putString(sendbuf);


  return RCFG_SUCCESS;


}




uint8_t rcfg_getGW(char * buf)
{
  ipAddress_t gw;
  ipEepromGetConfig(NULL, NULL, &gw);

  char sendbuf[15] = {0}; // clear buffer;
  ip2str(sendbuf, gw);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nGateway: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_getNet(char * buf)
{
  ipAddress_t ip, nm, gw;
  ipEepromGetConfig(&ip, &nm, &gw);

  char sendbuf[47] = {0}; // clear buffer;
  char * newpos;
  newpos = ip2str(sendbuf, ip);
  *newpos++ = ' ';
  newpos = ip2str(newpos, nm);
  *newpos++ = ' ';
  ip2str(newpos,gw);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nNIC-Overview: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_getDIP_Data(char *buf) {
  ipAddress_t dip;
  configEepromGetConfig(&dip, NULL, NULL, NULL, NULL);

  char sendbuf[15] = {0}; // clear buffer;
  ip2str(sendbuf, dip);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nDestination IP for Data-Channel: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_setDIP_Data(char *buf) {
  ipAddress_t dip = rcfg_parseIP(&buf);

  configEepromSetConfig(dip,0,0,0,0);

  return RCFG_SUCCESS;
}

uint8_t rcfg_getDPORT_Data(char *buf) {
  portAddress_t dport;
  configEepromGetConfig(NULL, &dport, NULL, NULL, NULL);

  char sendbuf[5] = {0}; // clear buffer;
  ultoa(dport, sendbuf, 10);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nDestination Port for Data-Channel: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_setDPORT_Data(char *buf) {
  portAddress_t dport = (portAddress_t)rcfg_parseUInt(&buf);

  configEepromSetConfig(0, dport, 0, 0, 0);

  return RCFG_SUCCESS;
}

uint8_t rcfg_getDIP_Config(char *buf) {
  ipAddress_t dip;
  configEepromGetConfig(NULL, NULL, &dip, NULL, NULL);

  char sendbuf[15] = {0}; // clear buffer;
  ip2str(sendbuf, dip);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nDestination IP for Config-Channel: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_setDIP_Config(char *buf) {
  ipAddress_t dip = rcfg_parseIP(&buf);

  configEepromSetConfig(0, 0, dip, 0, 0);

  return RCFG_SUCCESS;
}

uint8_t rcfg_getDPORT_Config(char *buf) {
  portAddress_t dport;
  configEepromGetConfig(NULL, NULL, NULL, &dport, NULL);

  char sendbuf[5] = {0}; // clear buffer;
  ultoa(dport, sendbuf, 10);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);
  
  putString("\r\nDestination Port for Config-Channel: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_setDPORT_Config(char *buf) {
  portAddress_t dport = (portAddress_t)rcfg_parseUInt(&buf);

  configEepromSetConfig(0, 0, 0, dport, 0);

  return RCFG_SUCCESS;
}

uint8_t rcfg_setBaud(char * buf)
{
	uint16_t prescaler=0;
	uint8_t parity=0;
	uint8_t stop_bits=0;
	uint8_t data_bits=0;

	serialEepromGetConfig(&prescaler,&parity,&stop_bits,&data_bits);
	uint32_t baud = rcfg_parseUInt(&buf);
	prescaler=F_CPU / (16 * baud) -1;
	serialEepromSetConfig( prescaler,parity,stop_bits,data_bits );
	return RCFG_SUCCESS;
}

uint8_t rcfg_getBaud(char * buf)
{
  uint16_t prescaler;
  serialEepromGetConfig(&prescaler,NULL,NULL,NULL);

  char sendbuf[10] = {0}; // clear buffer;


// BAUD = CLOCK_FREQUENCY / (16 * (UBBR + 1))
  //      = (CLOCK_FREQUENCY / 16) / (UBBR + 1)

  uint32_t baud=F_CPU  / (16 * (prescaler + 1));

  ultoa(baud, sendbuf, 10);

  rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

  putString("\r\nBaudrate for Data: ");
  putString(sendbuf);


  return RCFG_SUCCESS;
}

uint8_t rcfg_setParity(char * buf)
{
	uint16_t prescaler=0;
	uint8_t parity=0;
	uint8_t stop_bits=0;
	uint8_t data_bits=0;

	serialEepromGetConfig(&prescaler,&parity,&stop_bits,&data_bits);
	parity = rcfg_parseUInt(&buf);

	if(parity<=3)
	{
		serialEepromSetConfig( prescaler,parity,stop_bits,data_bits );
		return RCFG_SUCCESS;
	}
	else
		return RCFG_FAIL;
}
uint8_t rcfg_getParity(char * buf)
{

	uint8_t parity=0;

	char sendbuf[10] = {0}; // clear buffer;

	serialEepromGetConfig(NULL,&parity,NULL,NULL);
	ultoa(parity, sendbuf, 10);

	rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

	putString("\r\n");
	putString(sendbuf);

	switch (parity)
	{
		case 0:
			putString("\r\nParity Disabled: ");
		break;
		case 1:
			putString("\r\nParity Reserved: ");
		break;
		case 2:
			putString("\r\nParity Enabled, Even Parity: ");
		break;
		case 3:
			putString("\r\nParity Enabled, Odd Parity: ");
		break;
	}
	

  return RCFG_SUCCESS;
}
uint8_t rcfg_setStopBits(char * buf)
{
	uint16_t prescaler=0;
	uint8_t parity=0;
	uint8_t stop_bits=0;
	uint8_t data_bits=0;

	serialEepromGetConfig(&prescaler,&parity,&stop_bits,&data_bits);
	stop_bits = rcfg_parseUInt(&buf);

	if((stop_bits==1)||(stop_bits==2))
	{
		stop_bits-=1;
		serialEepromSetConfig( prescaler,parity,stop_bits,data_bits );
		return RCFG_SUCCESS;
	}
	else
		return RCFG_FAIL;


}
uint8_t rcfg_getStopBits(char * buf)
{

	uint8_t stop_bits=0;

	char sendbuf[10] = {0}; // clear buffer;

	serialEepromGetConfig(NULL,NULL,&stop_bits,NULL);
	ultoa(stop_bits+1, sendbuf, 10);

	rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

	putString("\r\nStopBits: ");
	putString(sendbuf);

  return RCFG_SUCCESS;
}
uint8_t rcfg_setDataBits(char * buf)
{
	uint16_t prescaler=0;
	uint8_t parity=0;
	uint8_t stop_bits=0;
	uint8_t data_bits=0;

	serialEepromGetConfig(&prescaler,&parity,&stop_bits,&data_bits);
	data_bits = rcfg_parseUInt(&buf);
	if((5<=data_bits)&&(data_bits<=8))
	{
		data_bits-=5;
		serialEepromSetConfig( prescaler,parity,stop_bits,data_bits );
		return RCFG_SUCCESS;
	}
	else
		return RCFG_FAIL;

  	
}
uint8_t rcfg_getDataBits(char * buf)
{

	uint8_t data_bits=0;
	char sendbuf[10] = {0}; // clear buffer;

	serialEepromGetConfig(NULL,NULL,NULL,&data_bits);
	data_bits+=5;
	ultoa(data_bits, sendbuf, 10);
	

	rtpSendCommand(sendbuf, stringLength(sendbuf), &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

	putString("\r\nDataBits: ");
	putString(sendbuf);

  return RCFG_SUCCESS;
}





uint8_t rcfg_Reboot(char * buf)
{
  // disable interrupts
  cli();

  // jump into bootloader
  asm volatile ("jmp 0x1e000");

  return RCFG_SUCCESS;
}

uint8_t rcfg_Disable(char * buf)
{
  isDisabled = 1;
  return RCFG_SUCCESS;
}

uint8_t Config_is_Disabled()
{
  return isDisabled;
}

#endif

