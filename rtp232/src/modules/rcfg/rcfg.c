#ifdef REQUIRES_RCFG

#include <stdio.h>
#include "../../config.h"
#include "commands.h"
#include "../rtp/rtp.h"

#include "./../../hal/com/com1_debug.h"


#include "rcfg.h"

typedef struct {
  char * name;
  uint8_t (*func)(char *);
} command_t;

command_t rcfg_commands[] = {
  {":SETMAC", rcfg_setMAC},
  {":GETMAC", rcfg_getMAC},

  {":SETIP", rcfg_setIP},
  {":SETNM", rcfg_setNM},
  {":SETGW", rcfg_setGW},
  {":SETNET", rcfg_setNet},
  {":GETIP", rcfg_getIP},
  {":GETNM", rcfg_getNM},
  {":GETGW", rcfg_getGW},
  {":GETNET", rcfg_getNet},

  {":SETBAUD", rcfg_setBaud},
  {":GETBAUD", rcfg_getBaud},
  {":SETPARITY", rcfg_setParity},
  {":GETPARITY", rcfg_getParity},  
  {":SETSTOPBITS", rcfg_setStopBits},
  {":GETSTOPBITS", rcfg_getStopBits},
  {":SETDATABITS", rcfg_setDataBits},
  {":GETDATABITS", rcfg_getDataBits},

  {":GETDIP_DATA", rcfg_getDIP_Data},
  {":SETDIP_DATA", rcfg_setDIP_Data},
  {":GETDPORT_DATA", rcfg_getDPORT_Data},
  {":SETDPORT_DATA", rcfg_setDPORT_Data},
  {":GETDIP_CONFIG", rcfg_getDIP_Config},
  {":SETDIP_CONFIG", rcfg_setDIP_Config},
  {":GETDPORT_CONFIG", rcfg_getDPORT_Config},
  {":SETDPORT_CONFIG", rcfg_setDPORT_Config},
  {":GETCONFIG", rcfg_getConfig},



  /*
  {":READEEPROM", rcfg_readEeprom},
  */
  {":REBOOT", rcfg_Reboot},
  {":DISABLE", rcfg_Disable},
  {"", NULL} // end-of-table -- do not delete this line!
};

void onRtpControl(char* buf, uint8_t len)
{



  // check if rcfg has been disabled using ":DISABLE?"
  if (Config_is_Disabled()) {
    rtpSendCommand("Config has been disabled.", 25, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);
    rtpSendCommand("FAIL\n", 5, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);
	putString("\r\nonRtpControl() Config_is_Disabled:");
    return;
  }


  // some syntax checks
  uint8_t pos = 1;
  while (pos < len)
  {
    if (buf[pos] == '?')
    {
      break;
    }
    pos++;
  }
  
 


  if (pos == len || buf[0] != ':')
  {
    putString("\r\nonRtpControl() Syntax Error,sent:");
	putString(buf);
    // '?' not found or ':' missing
    rtpSendCommand("Syntax Error\n", 13, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

	
    return;
  }
  
  
  uint8_t cmd=0;
  while (rcfg_commands[cmd].func != NULL) {
    pos = 0;
	// find command (case-insensitive => "& 0x5f")
    while ( ((rcfg_commands[cmd].name[pos] ^ buf[pos]) & 0x5f) == 0 && buf[pos] != 0x20) {
	  pos++;
	}
	if (rcfg_commands[cmd].name[pos] == 0 && (buf[pos]==' ' || buf[pos]=='?') ) {
	  // command found
      while(buf[pos] == ' ')
	    buf++;
      //rtpSendCommand(&rcfg_commands[cmd].name[0], 8, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort); // debug
      // execute command

      uint8_t rv;
      rv = rcfg_commands[cmd].func(&buf[pos]);
  


      // send ack
      if (rv == RCFG_SUCCESS ) 
	  {
        rtpSendCommand("OK\n", 3, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);
		putString("\r\nonRtpControl() meldet OK:");
      } 
	  else 
	  {
        rtpSendCommand("FAIL\n", 5, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);
		putString("\r\nonRtpControl() meldet FAIL:");
      }



	  return;
	}
	
	// compare to next command
	cmd++;
  }


  putString("onRtpControl() Command not found, sent:");
 putString(buf);
  // command not found
  rtpSendCommand("Error located between ears of user.\n", 36, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);    
  return;

  
}

#endif
