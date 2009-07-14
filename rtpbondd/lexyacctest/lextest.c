#include<stdio.h>
#include<ctype.h>

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

extern int devCounter;
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
