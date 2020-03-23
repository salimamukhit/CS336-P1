TARGET=./bin/client ./bin/server 
CC=gcc
CFLAGS= -Wall -Wextra -g
normal: $(TARGET)
client: client.c
    $(CC) $(CFLAGS) client.c -o client
server: server.c
    $(CC) $(CFLAGS) server.c -o server
clean:
    $(RM) $(TARGET) vgcore.*



# Output binary name
#bin=server

# Set the following to '0' to disable log messages:
#LOGGER ?= 1

# Compiler/linker flags
#CFLAGS += -g -Wall -lm -lreadline -fPIC -DLOGGER=$(LOGGER)
#LDFLAGS +=

#src=server.c
#obj=$(src:.c=.o)

#all: $(bin)

#$(bin): $(obj)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

#libshell.so: $(obj)
#	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -shared -o $@

#server.o: server.c logger.h next_token.h

#clean:
#	rm -f $(bin) $(obj) 
	