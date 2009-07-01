# Makefile for tty_net.c

EXEC_TTY_NET = tty_net
EXEC_ECHO_CLIENT = echo_client
EXEC_ECHO_SERVER = echo_server

OBJS_TTY_NET = tty_net.o
OBJS_ECHO_CLIENT = echo_client.o
OBJS_ECHO_SERVER = echo_server.o

SRC_TTY_NET = tty_net.c
SRC_ECHO_CLIENT = echo_client.c
SRC_ECHO_SERVER = echo_server.c

# change INCLDIR for your purposes
# INCLDIR = /usr/include
# INCLDIR = /usr/avr32-linux/include
	
		
all: $(EXEC_TTY_NET) $(EXEC_ECHO_CLIENT) $(EXEC_ECHO_SERVER)


$(EXEC_TTY_NET): $(OBJS_TTY_NET)
	$(CC) -I$(INCLDIR) -lpthread -Wl,-elf2flt -o $@ $(OBJS_TTY_NET) -lm

$(EXEC_ECHO_CLIENT): $(OBJS_ECHO_CLIENT)
	$(CC) -I$(INCLDIR) -lpthread -Wl,-elf2flt -o $@ $(OBJS_ECHO_CLIENT) -lm

$(EXEC_ECHO_SERVER): $(OBJS_ECHO_SERVER)
	$(CC) -I$(INCLDIR) -lpthread -Wl,-elf2flt -o $@ $(OBJS_ECHO_SERVER) -lm


$(OBJS_TTY_NET):
	$(CC) -c -I$(INCLDIR) $(SRC_TTY_NET)
	
$(OBJS_ECHO_CLIENT):
	$(CC) -c -I$(INCLDIR) $(SRC_ECHO_CLIENT)
	
$(OBJS_ECHO_SERVER):
	$(CC) -c -I$(INCLDIR) $(SRC_ECHO_SERVER)


clean:
	-rm -f $(EXEC_TTY_NET) $(EXEC_ECHO_CLIENT) $(EXEC_ECHO_SERVER) *.elf *.gdb *.o
		