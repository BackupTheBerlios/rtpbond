# Makefile for tty_net.c
EXEC_TTY_NET = tty_net
EXEC_TTY_NET_BF = tty_net_bf
OBJS_TTY_NET = tty_net.o
OBJS_TTY_NET_BF = tty_net_bf.o
SRC_TTY_NET = tty_net.c

#The configuration for the Linuxhost
LDFLAGS = -lpthread -o $@ -lm
INCLDIR = /usr/include

#The configuration for the DNP
CC_BF = /home/mike/opt/uClinux/bfin-uclinux/bin/bfin-uclinux-gcc
CFLAGS_BF = -Wl,-elf2flt
INCLDIR_BF = /home/mike/opt/uClinux/bfin-uclinux/bfin-uclinux/runtime/usr/include/
LIBDIR_BF = /home/mike/opt/uClinux/bfin-linux-uclibc/bfin-linux-uclibc/runtime/lib/
LDFLAGS_BF = -pthread -o $@ -lm -L$(LIBDIR_BF)
		
all: $(EXEC_TTY_NET) 


$(EXEC_TTY_NET): $(OBJS_TTY_NET)
	$(CC) -I$(INCLDIR) $(LDFLAGS) $(OBJS_TTY_NET) -lm -lc

$(OBJS_TTY_NET):
	$(CC) -c -I$(INCLDIR) $(SRC_TTY_NET)
	
dnp: $(EXEC_TTY_NET_BF)

$(EXEC_TTY_NET_BF): $(OBJS_TTY_NET_BF)
	$(CC_BF) -I$(INCLDIR_BF) $(LDFLAGS_BF) $(CFLAGS_BF) $(OBJS_TTY_NET_BF) -lm

$(OBJS_TTY_NET_BF):
	$(CC_BF) -c -o $@ -I$(INCLDIR_BF) $(CFLAGS_BF) $(SRC_TTY_NET)

clean:
	-rm -f $(EXEC_TTY_NET) $(EXEC_TTY_NET_BF) *.elf *.gdb *.o
