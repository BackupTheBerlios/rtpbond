/*
 * A small tool, that writes to/reads from a FIFO with or without blocking it.
 */

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>

void printUsage() {
    printf("usage: fifoio (rb|rn|wb|wn) FIFOname\n");
    exit(1);
}

int main(int argc, char* argv[]) {
    char buffer[256];
    int fifo;
    memset(buffer, 0, 256);
	scanf("%s", buffer);

    if(argc < 3)
        printUsage();

    if(argv[1][1] == 'r') {
        //read from fifo
        if(argv[1][2] == 'b')
            fifo = open(argv[2], O_RDONLY);
        else if(argv[1][2] == 'n')
            fifo = open(argv[2], O_RDONLY | O_NONBLOCK);
        else {
            printUsage();
            exit(1);
        }

        if(!fifo) {
            printf("error: cannot open %s\n", argv[2]);
            exit(1);
        }
        if(!read(fifo, buffer, 256))
            printf("error: cannot read from %s\n", argv[2]);
        printf("%s\n", buffer);
        close(fifo);
    }
    else if(argv[1][1] == 'w') {
	    scanf("%s", buffer);
        //write to fifo
        if(argv[1][2] == 'b')
            fifo = open(argv[2], O_WRONLY);
        else if(argv[1][2] == 'n')
            fifo = open(argv[2], O_WRONLY | O_NONBLOCK);
        else {
            printUsage();
            exit(1);
        }

        if(!fifo) {
            printf("error: cannot open %s\n", argv[2]);
            exit(1);
        }

        if(!write(fifo, buffer, 256))
            printf("error: cannot write to %s\n", argv[2]);
        close(fifo);
    }
}




    

