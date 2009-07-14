/*
name:			tty_net.c
author(s):		Iomsn Egenson, Mike Mueller
version:		0.15
description:	Creates a virtual tty device which communicates over UDP. Uses FIFOs.
history:		0.13:	- sends to a specific port and receives from it
						- creates CDP and RTP header
						- extracts CDP and RTP header
						- sends a "Im alive" package when time to last package exceeds TMax
						- adjustable remote IP, remote port, TMax and baud rate
						- multi threaded
						- uses FIFOs as virtual devices
				0.14:	- user guide was commented out for use with netserial
						- uses local sockets to communicate with netserial
						- improved error handling
                0.15:   - possibility to open several devices through configfile added
                        - "I'm alive package is not written to FIFO anymore
                        - only real payload-length is written to FIFO
                        
*/
		
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>
		
#define PL_SIZE 150
#define RTP_HEADER_SIZE 12
#define CDP_HEADER_SIZE 10
#define MAX_SN 65535

#define MAX_DEV 256

//Since strlcpy is not included in the glibc, here is the source:
//strncpy can't be used since it doesn't alsways NUL terminate.
/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t siz) {
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}


static char *help="usage: tty_net deviceName destIp4Addr destPort listenPort (0 for any) baudRate[bytes/s] TMax[ms] payloadType \nor: tty_net config-file\n";

// contains info about the device
struct netSerDev
{
	char name[256];
    char path[256];
	char destIp4Addr[15];	// IPv4
	int destPort;
	int listenPort;
	int baudRate;
	int TMax;
    char payloadType[3]; 
    int deviceNr;
} devConfig;

//contains statistics about the devices
struct devStats {
    clock_t TLastPkg; //time of last received packet
    uint32_t pkgDataSent; //number of datapackets sent
    uint32_t pkgAliveSent; //number of alivepackets sent
    uint32_t pkgDataRcvd; //number of datapackets received
    uint32_t pkgAliveRcvd; //number of alivepackets received
    uint32_t pkgLoss;
    uint32_t payloadSumSent;
    uint32_t payloadSumRcvd;
};

//the RTP header structure
struct soRtpHeader_t
{
	uint8_t version;
		// + version    : 2 
		// + padding    : 1
		// + extension  : 1
		// + CRSC Count : 4
	uint8_t type;
	uint16_t sequence;  // sequence number
	uint32_t timestamp;
	uint32_t ssrc;      // synchonizsation source identifier
};

//the CDP header structure
struct cdpHeader_t
{
	uint8_t length;
	uint8_t reserved;
	uint64_t timestamp;
};
		
pthread_t threads[MAX_DEV*2];	// receive and send threads	for all devices	
struct netSerDev devices[MAX_DEV]; //the deviceconfig
struct devStats devicestats[MAX_DEV]; //contains the infos about the devices
uint32_t startTime;	// time whe program is started (used as SSRC)
char cwd[256];


// appends the RTP header to bufferCdp and writes the result in bufferCdpRtp
void appendRtpHeader(char* bufferCdp, struct soRtpHeader_t RtpHeader, char* bufferCdpRtp)
{
	int i, arrayPos;
		
	// write the RtpHeader in bufferCdpRtp
	bufferCdpRtp[0] = RtpHeader.version;
	bufferCdpRtp[1] = RtpHeader.type;
	arrayPos = 2;
	for (i = arrayPos; i < sizeof(uint16_t) + arrayPos; i++)
		bufferCdpRtp[sizeof(uint16_t)+2*arrayPos-i-1] = RtpHeader.sequence >> 8*(i-arrayPos);
	arrayPos = i;				
	for (i = arrayPos; i < sizeof(uint32_t) + arrayPos; i++)
		bufferCdpRtp[sizeof(uint32_t)+2*arrayPos-i-1] = RtpHeader.timestamp >> 8*(i-arrayPos);
	arrayPos = i;
	for (i = arrayPos; i < sizeof(uint32_t) + arrayPos; i++)
		bufferCdpRtp[sizeof(uint32_t)+2*arrayPos-i-1] = RtpHeader.ssrc >> 8*(i-arrayPos);
	// write the payload + cdpHeader in bufferCdpRtp
	for (i = 0; i < CDP_HEADER_SIZE + PL_SIZE; i++)
		bufferCdpRtp[i+RTP_HEADER_SIZE] = bufferCdp[i];
	
}
		

// appends the CDP header to buffer and writes the result in bufferCdp
void appendCdpHeader(char* buffer, struct cdpHeader_t CdpHeader, char* bufferCdp)
{
	int i, arrayPos;
	
	// write the CdpHeader in bufferCdp
	bufferCdp[0] = CdpHeader.length;
	bufferCdp[1] = CdpHeader.reserved;
	arrayPos = 2;
	for (i = arrayPos; i < sizeof(uint64_t)+arrayPos; i++)
		bufferCdp[sizeof(uint64_t)+2*arrayPos-i-1] = CdpHeader.timestamp >> 8*(i-arrayPos);				
	// write the payload in bufferCdp
	for (i = 0; i < PL_SIZE; i++)
		bufferCdp[i+CDP_HEADER_SIZE] = buffer[i];
	
}
		

//extracts the RTP header from a given buffer und returns it
struct soRtpHeader_t retreiveRtpHeader(char *bufferCdpRtp)
{
	struct soRtpHeader_t RtpHeader;
	
	RtpHeader.version = bufferCdpRtp[0];
	RtpHeader.type = bufferCdpRtp[1];
	RtpHeader.sequence = bufferCdpRtp[2]*256 + bufferCdpRtp[3];
	RtpHeader.timestamp = bufferCdpRtp[4]*256*256*256 + bufferCdpRtp[5]*256*256 + bufferCdpRtp[6]*256 + bufferCdpRtp[7];
	
	return RtpHeader;
}
		

//extracts the CDP header from a given buffer and returns it
struct cdpHeader_t retreiveCdpHeader(char *bufferCdpRtp)
{
	struct cdpHeader_t CdpHeader;
	int i;
	
	
	CdpHeader.length = bufferCdpRtp[RTP_HEADER_SIZE];
    //if the payloadlength field contains a number > PL_SIZE it's not a valid CDP package.
    //we only take the first PL_SIZE bytes then
	if(CdpHeader.length > PL_SIZE)
		CdpHeader.length = PL_SIZE;

	CdpHeader.reserved = bufferCdpRtp[RTP_HEADER_SIZE+1];
	CdpHeader.timestamp = 0;
	for (i = 0; i < sizeof(uint64_t); i++)
		CdpHeader.timestamp += bufferCdpRtp[RTP_HEADER_SIZE+1+sizeof(uint64_t)-i] = 256*i;
	
	return CdpHeader;
}
		

// exit handler which closes sockets/FIFOs
void closeDev(void *arg)
{
	int *fd = (int *)arg;
	
	// close socket/FIFO
	close(*fd);
}


// exit handler which removes sockets/FIFOs
void removeDev(void *arg)
{
	char *name = (char *)arg;
	
	// remove socket/FIFO
	unlink(name);
}


//the receivefunction that receives from a given device, splits off the RTP- and CDP-header
//and writes the payload into a FIFO
void * recvRtp(void* deviceAttr)
{
    //get the device attributes
    struct netSerDev* attributes = (struct netSerDev*) deviceAttr;
    struct netSerDev devConf = *attributes;

    //reset the devicestatistics
    devicestats[devConf.deviceNr].pkgDataRcvd = 0;
    devicestats[devConf.deviceNr].pkgAliveRcvd = 0;
    devicestats[devConf.deviceNr].payloadSumRcvd = 0;
    devicestats[devConf.deviceNr].TLastPkg = 0;


    //construct local address structure
    struct sockaddr_in localAddress;
    memset(&localAddress, 0, sizeof(localAddress));		// zero out structure
    localAddress.sin_family = AF_INET;					// internet address family
    localAddress.sin_addr.s_addr = htonl(INADDR_ANY);	// any incoming interface
    localAddress.sin_port = htons(devConf.listenPort);		// listen port

    //create socket
    int socketRecv;
	socketRecv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketRecv < 0) {
		printf("tty_net: error: creating socket(s) failed\n");
		exit(1);
	}


	char buffer[PL_SIZE];	// payload
	char bufferCdpRtp[PL_SIZE + CDP_HEADER_SIZE + RTP_HEADER_SIZE];	// payload + cdpHeader + rtpHeader		
	int err, i;
	int inFifo;	// write payload in fifo
	struct cdpHeader_t cdpHeader; //the CDP header
	struct soRtpHeader_t rtpHeader; //the RTP header
	struct sockaddr_in remoteAddress;	// remote address
	unsigned int remoteAddrLength;
	char inFifoName[256+4];
    unsigned int lastSequenceNr = 0;
	// exit handlers
	pthread_cleanup_push(removeDev, (void *)&inFifoName);
	pthread_cleanup_push(closeDev, (void *)&inFifo);
	pthread_cleanup_push(closeDev, (void *)&socketRecv);
	
    //the name of the FIFO we write to is the given name, the payloadType + ".in"
	strcpy(inFifoName, devConf.name);
    strncat(inFifoName, ".", 1);
    strncat(inFifoName, devConf.payloadType, strlen(devConf.payloadType));
	strncat(inFifoName, ".in", 3);

	// set up FIFO.in
    chdir(cwd);
    mkdir(devConf.path, 0777);
    chdir(devConf.path);
    //delete possibly existing FIFO with same name
    unlink(inFifoName);
    // create FIFO
    err = mkfifo(inFifoName, 0666);
    if(err < 0)
    {
        printf("tty_net: error: cannot create %s in %s. ", inFifoName, devConf.path);
        if(errno == 13)
            printf("Permission denied.\n");
        else
            printf("\n");
        exit(1);
    }
    //open it
	inFifo = open(inFifoName, O_RDWR | O_NONBLOCK);
	if (inFifo < 0)
	{
        printf("tty_net: error: cannot open %s\n", inFifoName);
	}
    
	// bind socket to the local address
    //unlink(socketRecv);
	err = bind(socketRecv, (struct sockaddr *) &localAddress, sizeof(localAddress));
    if ((err) < 0)
	{		
        printf("tty_net: error: bind(socketRecv) failed. ");
        if(errno == 98) 
            printf("Socket already in use!\n");
        else
            printf("\n");
		exit(1);
	}
	
	// receive data from the net and write it into FIFO
	while(1)
	{
		remoteAddrLength = sizeof(remoteAddress);
		// clear buffer
		memset(bufferCdpRtp, 0, PL_SIZE + RTP_HEADER_SIZE + CDP_HEADER_SIZE);
		
		// retreive data from socket
		err = recvfrom(socketRecv, bufferCdpRtp, PL_SIZE + CDP_HEADER_SIZE + RTP_HEADER_SIZE, 0, (struct sockaddr *) &remoteAddress, &remoteAddrLength);
				
		if (err > 0)
		{		
			// split off the RtpHeader
			rtpHeader = retreiveRtpHeader(bufferCdpRtp);
			// split off the CdpHeader
			cdpHeader = retreiveCdpHeader(bufferCdpRtp);			
			// split off the payload
			for (i = 0; i < cdpHeader.length; i++)
				buffer[i] = bufferCdpRtp[i + CDP_HEADER_SIZE + RTP_HEADER_SIZE];
			
			// write into FIFO
			if (cdpHeader.length > 0 && rtpHeader.type == atoi(devConf.payloadType))	// incoming data
				write(inFifo, buffer, cdpHeader.length);
            //update the statistics
            devicestats[devConf.deviceNr].TLastPkg = clock();
            if(cdpHeader.length != 0)
                devicestats[devConf.deviceNr].pkgDataRcvd++;
            else
                devicestats[devConf.deviceNr].pkgAliveRcvd++;
            devicestats[devConf.deviceNr].payloadSumRcvd += cdpHeader.length;

            //check if the sequenceNr is lastSequenceNr + 1, if we already got one package
            if(devicestats[devConf.deviceNr].pkgDataRcvd > 1 || devicestats[devConf.deviceNr].pkgAliveRcvd > 1 ) 
                if(rtpHeader.sequence != lastSequenceNr + 1)
                    devicestats[devConf.deviceNr].pkgLoss++;

            //update the last sequence number
            lastSequenceNr = rtpHeader.sequence;
            
		}			
		if (err < 0)
			printf("tty_net: error: cannnot read from socket\n");
	}
	
	close(inFifo);
	close(socketRecv);
	
	pthread_cleanup_pop(1);	
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);	
    
    return 0;
}
		
		
//the sendfunction that reads data from a FIFO, wraps the CDP and RTP headers around
//and writes the package into a given socket
void * sendRtp(void* deviceAttr)
{
    //get the devices attributes
    struct netSerDev* attributes = (struct netSerDev*) deviceAttr;
    struct netSerDev devConf = *attributes;
    int destAddrLength;

    //reset the statistics
    devicestats[devConf.deviceNr].pkgDataSent = 0;
    devicestats[devConf.deviceNr].pkgAliveSent = 0;
    devicestats[devConf.deviceNr].payloadSumSent = 0;

    //destination address structure
    struct sockaddr_in destAddress;
	destAddress.sin_family = AF_INET;
	destAddress.sin_addr.s_addr = inet_addr(devConf.destIp4Addr);
	destAddress.sin_port = htons(devConf.destPort);
	destAddrLength = sizeof(destAddress);

    //create the socket
    int socketSend;
	socketSend = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketSend < 0) {
		printf("tty_net: error: creating socket(s) failed\n");
		exit(1);
	}


	char buffer[PL_SIZE];	// payload
	char* bufferCdp = malloc(PL_SIZE + CDP_HEADER_SIZE);	// payload + cdpHeader
	char* bufferCdpRtp = malloc(PL_SIZE + CDP_HEADER_SIZE + RTP_HEADER_SIZE);	// payload + cdpHeader + rtpHeader
	int err;
	int outFifo;	// read the payload from here
	struct cdpHeader_t cdpHeader; //the CDP header
	struct soRtpHeader_t rtpHeader; //the RTP header
	int sn;	// sequence number
	clock_t tLastSent = 0;	// time when last data package was sent
	char outFifoName[256+4]; 
	
	// exit handlers
	pthread_cleanup_push(removeDev, (void *)&outFifoName);
	pthread_cleanup_push(closeDev, (void *)&outFifo);
	pthread_cleanup_push(closeDev, (void *)&socketSend);
	
    //the name of the FIFO we read from is the given name, the payloadType + ".out"
	strcpy(outFifoName, devConf.name);
    strncat(outFifoName, ".", 1);
    strncat(outFifoName, devConf.payloadType, strlen(devConf.payloadType));
	strncat(outFifoName, ".out", 4);
	// set up outFifo
    chdir(cwd);
    mkdir(devConf.path, 0777);
    chdir(devConf.path);
    //delete possibly existing old FIFO with same name
    unlink(outFifoName);
    //create the FIFO
	err = mkfifo(outFifoName, 0666);
    if(err) {
        printf("tty_net: error: cannot create %s in %s\n", outFifoName, devConf.path);
        exit(1);
    }
	outFifo = open(outFifoName, O_RDONLY | O_NONBLOCK);
	if (outFifo < 0)
	{
		printf("tty_net: error: cannot open FIFO %s\n", outFifoName);
		exit(1);
	}
	
    //the sequencenumber of the RTP package
	sn = 0;
	// empty the buffers
	memset(buffer, 0, PL_SIZE);
	memset(bufferCdp, 0, CDP_HEADER_SIZE + PL_SIZE);
	memset(bufferCdpRtp, 0, RTP_HEADER_SIZE + CDP_HEADER_SIZE + PL_SIZE);
	
	while(1)
	{		
		// read data from FIFO
		err = read(outFifo, buffer, PL_SIZE);
	/*	if (err < 0)
		{
			printf("tty_net: error: cannot read from %s\n", outFifoName);
			exit(1);
		}
	*/	
		// send package when we got real data or send empty package after timeout Tmax
		if ((err > 0) || difftime(clock(), tLastSent)/CLOCKS_PER_SEC >= devConf.TMax/1000.0)
		{		
			// make rtp header
			rtpHeader.version = 2*64 + 0*32;	// Version 2 + Padding 0
			rtpHeader.type = atoi(devConf.payloadType);	// the configured payloadType
			rtpHeader.sequence = sn;			// sequence number
			rtpHeader.timestamp = sn*PL_SIZE;
			
			// make cdp header
			cdpHeader.length = strlen(buffer);
			cdpHeader.timestamp = difftime(clock(),startTime);
			cdpHeader.timestamp = cdpHeader.timestamp << 4*8;
			
            //reset serialnumber if too large
			if (sn == MAX_SN)
				sn = 0;
			else
				sn++;
			
			// in case of timeout send "Im alive"			
			if (err == 0) {
                strncpy(buffer, "Im alive", 8);
				//buffer[0] = 'I'; buffer[1] = 'm'; buffer[2] = ' '; buffer[3] = 'a';
				//buffer[4] = 'l'; buffer[5] = 'i'; buffer[6] = 'v'; buffer[7] = 'e';
				cdpHeader.length = 0;
			}
									 
			// append headers Pay attention to the order!
			appendCdpHeader(buffer, cdpHeader, bufferCdp);
			appendRtpHeader(bufferCdp, rtpHeader, bufferCdpRtp);
		
			// wait if last package was sent too recently (baud rate)
			while (difftime(clock(), tLastSent)/CLOCKS_PER_SEC <= (double) PL_SIZE/devConf.baudRate)
			{
				 // wait
			}
			
			// send data through socket
			err = sendto(socketSend, bufferCdpRtp, RTP_HEADER_SIZE + CDP_HEADER_SIZE + PL_SIZE, 0, (struct sockaddr *) &destAddress, destAddrLength);
			if (err < 0)
			{
				printf("error: cannot send data\n");
				exit(1);
			}
            //update statistics
            if(cdpHeader.length != 0)
                devicestats[devConf.deviceNr].pkgDataSent++;
            else
                devicestats[devConf.deviceNr].pkgAliveSent++;

            devicestats[devConf.deviceNr].payloadSumSent += cdpHeader.length;
			
			tLastSent = clock();
			
			// empty the buffer
			memset(buffer, 0, PL_SIZE);			
		}		
	}
	
	close(outFifo);
	close(socketSend);
	
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	return 0;
}

//prints out an error message
void badConfig(int lineNumber)
{
	printf("tty_net: error: there is something wrong with the config file in line %i\n", lineNumber);
	exit(1);
}


//reads a given configfile
//returns the numbers of devices started
int readConf(char *fileName)
{	
    int err=0;
	FILE *pConfigFile;	// file descriptor of the config file
	char line[256];		// line being read from config file
	char nextDev = 1;	// determines if the next device should be found
	int lineNumber = 0;
	char *qmFirst, *qmLast;	// position of quotation marks
	char *optPos;	// position of the option name in the line
	int devCounter = 0;
    int optionCounter;
	char tmpStr[10];	//temporary string to be converted into int
	
	// open config file
	pConfigFile = fopen(fileName, "r");
	if (pConfigFile == NULL)
	{
		perror("error: could not open config file\n");
		exit(1);
	}
	
	while (!feof(pConfigFile))
	{
		// find the next device
		while ((nextDev == 1) && (!feof(pConfigFile)))
		{
			fgets(line, 256, pConfigFile);
			lineNumber++;
		
			if (line[0] == '#')	
			{ // comment, do nothing
			}
			else
			{
                //find the position of the string "deviceName"
				optPos = strstr(line, "deviceName");
				
				if (optPos > 0)	// device found
				{
					// find the quotation marks that enclose the argument
					qmFirst = strchr(line, '"');
					qmLast = strrchr(line, '"');
			
                    //if parameter has been found
					if ((qmFirst != 0) && (qmLast != 0) && (qmFirst != qmLast))
					{
				
						if (optPos > qmFirst)
							badConfig(lineNumber);
			         
                        //copy the devicename into the name field
                        strlcpy(devices[devCounter].name, qmFirst+1, qmLast-qmFirst);
                        //set the deviceNr
                        devices[devCounter].deviceNr = devCounter;

                        //continue to read the config for this device
						nextDev = 0;
					}
					else
						badConfig(lineNumber);
				}
			}
		}
		// get the config to this device		
		optionCounter = 0;
		while ((nextDev == 0) && (!feof(pConfigFile)))
		{
            //read a line
			fgets(line, 256, pConfigFile);
			lineNumber++;
		
			if (line[0] == '#')	
			{ // comment, do nothing
			}
			else
			{
				// find the quotation marks that enclose the argument
				qmFirst = strchr(line, '"');
				qmLast = strrchr(line, '"');
			
				if ((qmFirst != 0) && (qmLast != 0) && (qmFirst != qmLast))
				{
                    //read the path of the FIFOs
					if ((optPos = strstr(line, "path")) > 0)
					{
						if (optPos > qmFirst)
							badConfig(lineNumber);					
						optionCounter++;
						strlcpy(devices[devCounter].path, qmFirst+1, qmLast-qmFirst);
					}

                    //read the IP address of the destination device
					if ((optPos = strstr(line, "destIp4Addr")) > 0)
					{
						if (optPos > qmFirst)
							badConfig(lineNumber);					
						optionCounter++;
						strlcpy(devices[devCounter].destIp4Addr, qmFirst+1, qmLast-qmFirst);
					}
                    //read the portnumber of the destination device
					else if ((optPos = strstr(line, "destPort")) > 0)
					{
						if (optPos > qmFirst)
							badConfig(lineNumber);					
						optionCounter++;
						memcpy(tmpStr, qmFirst+1, (qmLast-qmFirst-1));
						devices[devCounter].destPort = atoi(tmpStr);
						memset(tmpStr, 0, 10);
					}
                    //read the portnumber to listen to on the local device
					else if ((optPos = strstr(line, "listenPort")) > 0)
					{
						if (optPos > qmFirst)
							badConfig(lineNumber);					
						optionCounter++;
						memcpy(tmpStr, qmFirst+1, (qmLast-qmFirst-1));
						devices[devCounter].listenPort = atoi(tmpStr);
						memset(tmpStr, 0, 10);
					}
                    //read the maximal baudrate to use
					else if ((optPos = strstr(line, "baudRate[bytes/s]")) > 0)
					{
						if (optPos > qmFirst)
							badConfig(lineNumber);					
						optionCounter++;
						memcpy(tmpStr, qmFirst+1, (qmLast-qmFirst-1));
						devices[devCounter].baudRate = atoi(tmpStr);
						memset(tmpStr, 0, 10);
					}
                    //read the maximal time between two packages.
					else if ((optPos = strstr(line, "TMax[ms]")) > 0)
					{
						if (optPos > qmFirst)
							badConfig(lineNumber);					
						optionCounter++;
						memcpy(tmpStr, qmFirst+1, (qmLast-qmFirst-1));
						devices[devCounter].TMax = atoi(tmpStr);
						memset(tmpStr, 0, 10);
					}
                    //read the payloadtype
					else if ((optPos = strstr(line, "payloadType")) > 0)
					{
						if (optPos > qmFirst)
							badConfig(lineNumber);					
						optionCounter++;
						strlcpy(devices[devCounter].payloadType, qmFirst+1, qmLast-qmFirst);
					}

					if (optionCounter == 7)	// got all options
					{
						devCounter++;
                        nextDev = 1;
					}
				}				
			}
		}
	}

    if(nextDev == 0) {
        printf("tty_net: error in configfile. Device %s is not correctly configured! Aborting\n", devices[devCounter].name);
        exit(1);
    }

	
	fclose(pConfigFile);
    return devCounter;
	
}


//starts the n devices configured in devices[0]...devices[n-1]
void startDevices(int n) {
    int err=0;
    while(n--) {
        //start the recv-thread for this device
        err = pthread_create(&threads[2*n], NULL, recvRtp, (void*) &devices[n]);
        if (err) {
            printf("tty_net: error: creating recvRtp for device %d failed\n", n);
            exit(1);
        }
        //start the send-thread for this device
        err = pthread_create(&threads[2*n+1], NULL, sendRtp, (void*) &devices[n]);
        if (err) {
            printf("tty_net: error: creating sendRtp for device %d failed\n", n);
            exit(1);
        }
    }
}

void restartDevice(char* name, char* filename) {
    int err=0;
    //read the config again
    int running = readConf(filename);
    while(running--) {
        //if the name of the device matches the given one, restart it
        if(!strcmp(devices[running].name, name)) {
            //cancel the old recv-thread for this device
            err = pthread_cancel(threads[2*running]);
            if(err) {
                printf("tty_net: error: canceling recvRtp for device %d failed\n", running);
                exit(1);
            }
            //cancel the old send-thread for this device
            err = pthread_cancel(threads[2*running+1]);
            if(err) {
                printf("tty_net: error: canceling sendRtp for device %d failed\n", running);
                exit(1);
            }
            //start the new send-thread for this device
            err = pthread_create(&threads[2*running+1], NULL, sendRtp, (void*) &devices[running]);
            if (err) {
                printf("tty_net: error: creating sendRtp for device %d failed\n", running);
                exit(1);
            }
            //start the new recv-thread for this device
            err = pthread_create(&threads[2*running], NULL, recvRtp, (void*) &devices[running]);
            if (err) {
                printf("tty_net: error: creating recvRtp for device %d failed\n", running);
                exit(1);
            }
        }
    }
}

void setStartDevice(int argc, char *argv[]) {
    //set the devicedata given by the commandlinearguments
    strcpy(devices[0].name, argv[1]);
    strcpy(devices[0].destIp4Addr, argv[2]);
    devices[0].destPort = atoi(argv[3]);
    devices[0].listenPort = atoi(argv[4]);
    devices[0].baudRate = atoi(argv[5]);
    devices[0].TMax = atoi(argv[6]);
    strcpy(devices[0].payloadType, argv[7]);
}

void printStatus(char* configFile, int runningDevices, clock_t actualClock) {
    int pkgSent, pkgRcvd, i;
    float avgplrcvd, avgplsent;
    //print the name of the configfile
    printf("Configfile loaded: %s\n", configFile);
    //print all the statusmessages
    printf("%i device(s) running:\n", runningDevices);
    for(i=0; i < runningDevices; i++) {
        //the total number of packets sent
        pkgSent = devicestats[i].pkgDataSent + devicestats[i].pkgAliveSent;
        //the total number of packets received
        pkgRcvd = devicestats[i].pkgDataRcvd + devicestats[i].pkgAliveRcvd;
        printf("%s:\n", devices[i].name);
        printf("\tpath: %s\n", devices[i].path);
        printf("\tdestIp4Addr: %s\tdestPort: %i\n", devices[i].destIp4Addr, devices[i].destPort);
        printf("\tlistenPort: %i\n", devices[i].listenPort);
        printf("\tbaudRate: %i\tTMax: %i\n", devices[i].baudRate, devices[i].TMax);
        printf("\tpayloadType: %s\n", devices[i].payloadType); 
        printf("\tPackets sent: %i (%i data/%i alive)", pkgSent, devicestats[i].pkgDataSent, devicestats[i].pkgAliveSent);
        printf("\tPackets received: %i (%i data/%i alive)", pkgRcvd, devicestats[i].pkgDataRcvd, devicestats[i].pkgAliveRcvd);
        printf("\tstatus: ");
        if(difftime(actualClock, devicestats[i].TLastPkg)/CLOCKS_PER_SEC >= devices[i].TMax/1000.0)
            printf("inactive\n");
        else
            printf("active\n");

        //calculate the average payloadlength
        if(pkgSent > 0)
            avgplsent = (float) devicestats[i].payloadSumSent / (float) pkgSent;
        else
            avgplsent = 0.0;
        if(pkgRcvd > 0)
            avgplrcvd = (float) devicestats[i].payloadSumRcvd / (float) pkgRcvd;
        else
            avgplrcvd = 0.0;

        printf("\tAvg payload sent: %3.2f byte\t Avg payload received: %3.2f byte\n", avgplsent, avgplrcvd);
        printf("\tPackets lost: %i\n", devicestats[i].pkgLoss);
    }
}


//the main function.
int main(int argc, char *argv[]) {
	pthread_t threads[MAX_DEV*2];	// receive and send threads
	int err, i;
	char cmd[256+8];	// command from other tty_net process
    char configFile[256];
	int localSocketIn, localSocketOut; // socket to communicate with other tty_net process (kind of IPC)
	struct sockaddr_un inAddress, outAddress;
	unsigned int inAddressLength, outAddressLength;
    struct netSerDev recvDevice[1];
    struct devStats recvStats[1];
    int runningDevices = 0;
    clock_t actualClock = 0;
	
    //check the number of given commandlinearguments
	if (argc > 3 && argc < 8) {
		printf("%s", help);
		return -1;
	}
	
	startTime = clock();
	srand(time(NULL));	// random seed numbers for the ssrc
				
    //CLI still works, so that old scripts are still usable
    if(argc >= 8) {
        //get startdevicedata
        setStartDevice(argc, argv);
        startDevices(1);
        runningDevices = 1;
    }

    //if the first parameter is status
    //handle a status request
    else if (!strcmp(argv[1], "status")) {
        //create the socket, with whom we send the command to the other process
        if ((localSocketOut = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
        {
            perror("tty_net: error: could not open socketOut\n");
            exit(1);
        }
        //create the socket, from which we get the status
        if ((localSocketIn = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
        {
            perror("tty_net: error: could not open socketIn\n");
            exit(1);
        }
        // if opening socket fails -> device does not exist or opening just failed
            
        //the socket which we use to request the status
        outAddress.sun_family = AF_LOCAL;
        strncpy(outAddress.sun_path, "ttynet.in", 11);
        outAddressLength = strlen(outAddress.sun_path) + sizeof(outAddress.sun_family) + 1;
        //the socket where we get the status from
        inAddress.sun_family = AF_LOCAL;
        strlcpy(inAddress.sun_path, "ttynet.ou", 11);
        unlink(inAddress.sun_path);
        inAddressLength = strlen(inAddress.sun_path) + sizeof(inAddress.sun_family) + 1;	
        bind(localSocketIn, (struct sockaddr*)&inAddress, inAddressLength);
         
        //get all parameters (send "getstatus" to the other process through the socket)
        strncpy(cmd, "getstatus", 9);
        if (sendto(localSocketOut, cmd, strlen(cmd), 0, (struct sockaddr *) &outAddress, outAddressLength) < 0)
        {
            printf("tty_net: error: status request failed. ");
            if(errno == 61) printf("tty_net not running!\n");
            else printf("%d", errno);
            return 1;
        }
        
        //receive config
        //receive the clock value
        recvfrom(localSocketIn, &actualClock, sizeof(clock_t), 0, (struct sockaddr *) &inAddress, &inAddressLength);
        //receive the name of the configfile
        recvfrom(localSocketIn, configFile, 256, 0, (struct sockaddr *) &inAddress, &inAddressLength);
        //receive the number of devices first
        recvfrom(localSocketIn, &runningDevices, sizeof(int), 0, (struct sockaddr *) &inAddress, &inAddressLength);

        //receive the status of every device and save it in the arrays
        for(i=0; i< runningDevices; i++) {
            recvfrom(localSocketIn, recvDevice, sizeof(devices[i]), 0, (struct sockaddr *) &inAddress, &inAddressLength);
            recvfrom(localSocketIn, recvStats, sizeof(devices[i]), 0, (struct sockaddr *) &inAddress, &inAddressLength);
            devices[i] = recvDevice[0];
            devicestats[i] = recvStats[0];
        }
        //print the statusmessages
        printStatus(configFile, runningDevices, actualClock);
        return 0;

    
    }
    //handle a restart request
    else if (!strcmp(argv[1], "restart")) {
        if(argc < 3) {
            printf("usage: tty_net restart deviceName\n");
            exit(1);
        }
        //create the socket, with whom we send the command to the other process
        if ((localSocketOut = socket(AF_LOCAL, SOCK_DGRAM, 0)) == -1)
        {
            perror("tty_net: error: could not open socketOut\n");
            exit(1);
        }
        //the socket which we use to request the status
        outAddress.sun_family = AF_LOCAL;
        strncpy(outAddress.sun_path, "ttynet.in", 11);
        outAddressLength = strlen(outAddress.sun_path) + sizeof(outAddress.sun_family) + 1;
        //restart the device (send "restart" to the other process through the socket)
        if (sendto(localSocketOut, argv[1], strlen(argv[1]), 0, (struct sockaddr *) &outAddress, outAddressLength) < 0)
        {
            printf("tty_net: error: restart request failed\n");
            return 1;
        }
        //send the name of the device we want to restart
        if (sendto(localSocketOut, argv[2], strlen(argv[2]), 0, (struct sockaddr *) &outAddress, outAddressLength) < 0)
        {
            printf("tty_net: error: restart request failed\n");
            return 1;
        }
    }

    else {
        //set cwd
        getcwd(cwd, 256);

        //read the configfile and start the devices
        runningDevices = readConf(argv[1]);
        startDevices(runningDevices);
        //change back to cwd
        chdir(cwd);
	    
        //create local sockets for communication with other tty_net process that calls status
        //the socket from where we read the command from other processes
	    if ((localSocketIn = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
        {
            printf("tty_net: creating localSocketIn failed\n");
            exit(1);
        }
        //the socket where we write the config to
        if ((localSocketOut = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
        {
            printf("tty_net: creating localSocketOut failed\n");
            exit(1);
        }

        //the socket from where we get the commands
        inAddress.sun_family = AF_UNIX;
        strncpy(inAddress.sun_path, "ttynet.in", 11);
        unlink(inAddress.sun_path);
        inAddressLength = strlen(inAddress.sun_path) + sizeof(inAddress.sun_family) + 1;
        //bind the inputsocket
        if (bind(localSocketIn, (struct sockaddr *) &inAddress, inAddressLength) < 0)
        {
            printf("tty_net: error: binding local socket failed! Possibly another tty_net process running already!\n");
            exit(1);
        }
        
        // waiting for command from other tty_net process
        cmd[0] = 0;
        while (cmd[0] == 0)
        {
            //receive a command from the local socket
            if (recvfrom(localSocketIn, cmd, 256+8, 0, (struct sockaddr*) &inAddress, &inAddressLength) < 0)
            {
                printf("tty_net: error: receiving from local socket failed\n");
                exit(1);
            }
            //if the command is "getstatus", send the config and status of all devices to the other process
            if (strstr(cmd, "getstatus"))
            {
                outAddress.sun_family = AF_LOCAL;
                strncpy(outAddress.sun_path, "ttynet.ou", 11);
                outAddressLength = strlen(outAddress.sun_path) + sizeof(outAddress.sun_family) + 1;

                //send the current clockvalue to other process
                actualClock = clock();
                sendto(localSocketOut, &actualClock, sizeof(clock_t), 0, (struct sockaddr *) &outAddress, outAddressLength);

                //send the name of the configfile to other process
                sendto(localSocketOut, argv[1], strlen(argv[1])+1, 0, (struct sockaddr *) &outAddress, outAddressLength);
                
                //send number of devices to other process
                sendto(localSocketOut, &runningDevices, sizeof(int), 0, (struct sockaddr *) &outAddress, outAddressLength);
                //send config and stats to other process
                for(i=0; i < runningDevices; i++) {
                    //send deviceconf
                    err = sendto(localSocketOut, (struct netSerDev *) &devices[i], sizeof(devices[i]), 0, (struct sockaddr *) &outAddress, outAddressLength);
                    //send devicestats
                    err = sendto(localSocketOut, (struct devStats *) &devicestats[i], sizeof(devicestats[i]), 0, (struct sockaddr *) &outAddress, outAddressLength);
                }

                cmd[0] = 0;
            }
            
            else if (strstr(cmd, "restart"))
            {
                //get the name of the device to restart
                i = recvfrom(localSocketIn, cmd, 256+8, 0, (struct sockaddr*) &inAddress, &inAddressLength);
                //nullterminate the string
                cmd[i] = 0;
                //restart it
                printf("Restarting %s from file %s\n", cmd, argv[1]);
                restartDevice(cmd, argv[1]);
                
                cmd[0] = 0;
            }
            
        }
    }

    for(i=0; i < runningDevices*2; i++)
        pthread_cancel(threads[i]);
	
    close(localSocketIn);
	unlink(inAddress.sun_path);
		
	pthread_exit(NULL);	
	return 0;
}
