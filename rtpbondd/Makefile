# Makefile for rtpbondd.c
EXEC_RTPBONDD = rtpbondd
EXEC_RTPBONDD_BF = rtpbondd_bf
OBJS_RTPBONDD = rtpbondd.o
OBJS_RTPBONDD_BF = rtpbondd_bf.o
SRC_RTPBONDD = rtpbondd.c

#The configuration for the Linuxhost
LDFLAGS = -lpthread -o $@ -lm
INCLDIR = /usr/include

#The configuration for the DNP
CC_BF = /home/mike/opt/uClinux/bfin-uclinux/bin/bfin-uclinux-gcc
CFLAGS_BF = -Wl,-elf2flt
INCLDIR_BF = /home/mike/opt/uClinux/bfin-uclinux/bfin-uclinux/runtime/usr/include/
LIBDIR_BF = /home/mike/opt/uClinux/bfin-linux-uclibc/bfin-linux-uclibc/runtime/lib/
LDFLAGS_BF = -pthread -o $@ -lm -L$(LIBDIR_BF)
		
all: $(EXEC_RTPBONDD) 


$(EXEC_RTPBONDD): $(OBJS_RTPBONDD)
	$(CC) -I$(INCLDIR) $(LDFLAGS) $(OBJS_RTPBONDD) -lm -lc

$(OBJS_RTPBONDD):
	$(CC) -c -I$(INCLDIR) $(SRC_RTPBONDD)
	
dnp: $(EXEC_RTPBONDD_BF)

$(EXEC_RTPBONDD_BF): $(OBJS_RTPBONDD_BF)
	$(CC_BF) -I$(INCLDIR_BF) $(LDFLAGS_BF) $(CFLAGS_BF) $(OBJS_RTPBONDD_BF) -lm

$(OBJS_RTPBONDD_BF):
	$(CC_BF) -c -o $@ -I$(INCLDIR_BF) $(CFLAGS_BF) $(SRC_RTPBONDD)

clean:
	-rm -f $(EXEC_RTPBONDD) $(EXEC_RTPBONDD_BF) *.elf *.gdb *.o
