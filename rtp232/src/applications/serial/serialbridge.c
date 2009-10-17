#ifdef APPLICATION_SERIALBRIDGE
  #include "./serialbridge.h"
  #include "./../../modules/serial/serial.h"
  #include "./../../modules/rtp/rtp.h"
  #include "./../../toolkit/netlib/netlib.h"
  #include "./../../config.h"
  #include "./../../hal/com/com1_debug.h"

  void onInitApplication()
  {
      return;
  }


  void onRtpData(char* buf, uint8_t len)
  {    
    if (len > 0)
      serialSendBytes(buf,len);


    return;
  }
  
  uint8_t onSerialData(char* buf, uint8_t len)
  {    


	//hier perfomance test vom enc hat funktioniert!!!! alx
    //rtpSendCommand("Syntax Error\n", 13, &Config.IPv4.remoteAdrConfig, &Config.IPv4.localSrcPort);

	//rtpSendCommand("egal\n", 6, &Config.IPv4.remoteAdrData, &Config.IPv4.localSrcPort);  


  

    rtpSendCDPData(buf, len, &Config.IPv4.remoteAdrData, &Config.IPv4.localSrcPort);  
	//putString_com1("\r\nSP");

    return 0;
  }
#endif
