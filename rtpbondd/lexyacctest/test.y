%{
#include<string.h>
#include<stdio.h>
%}
%start zeile
%token DEVICENAME DESTIPADDR DESTPORT LISTENPORT BAUDRATE TMAX PAYLOADTYPE ARGUMENT ZE 

%%
devs : device
        ;

device : zeile
       ;

zeile : DEVICENAME ZE { printf("aaa\n"); devCounter++; strcpy(devices[devCounter].name, argument); }
      ;
%%
#include "lex.yy.c"
#include<stdio.h>
#include<ctype.h>

int devCounter = 0;

struct netSerDev
{
	char name[256];
	char destIp4Addr[15];	// IPv4
	int destPort;
	int listenPort;
	int baudRate;
	int TMax;
    char payloadType[3]; 
    int deviceNr;
};
struct netSerDev devices[25]; 

extern char argument[256];
int main(int argc, char* argv[]) {

    int tok;
    freopen(argv[1], "r", stdin);
    printf("foo\n");
    yyparse();
    printf("%d\n", devCounter);
    while(devCounter--) {
        
       printf("%s", devices[devCounter].name);
    }
} 
void yyerror(char *s) {
    printf("%s", s);
}

