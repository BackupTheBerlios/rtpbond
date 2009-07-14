%{
char argument[256];
#include<stdio.h>
#include<string.h>

%}
%%
[ \t]*              ;
\n                  return(ZE);
deviceName          return(DEVICENAME);
destIp4Addr         return(DESTIPADDR);
destPort            return(DESTPORT);
listenPort          return(LISTENPORT);
baudRate\[bytes\/s\] return(BAUDRATE);
TMax\[ms\]          return(TMAX);
payloadType         return(PAYLOADTYPE);
.              {strcpy(argument, yytext); return(ARGUMENT);}
%%

