# Set the following to '0' to disable log messages:
LOGGER ?= 1

TARGET=client server
CC=gcc
CFLAGS += -g -Wall -lm -fPIC -DLOGGER=$(LOGGER) -lreadline

# Source C files
#globalsrc= ini_parser.c next_token.c
serversrc= server.c tcp_server.c udp_server.c
clientsrc= client.c tcp_client.c udp_client.c
src=$(serversrc) $(clientsrc)

obj=$(src:.c=.o)

$(TARGET): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

# Individual dependencies --
server.o: server.c tcp_server.h udp_server.h logger.h
tcp_server.o: tcp_server.c logger.h
udp_server.o: udp_server.c logger.h

client.o: tcp_client.c tcp_client.h udp_client.h logger.h
tcp_client.o: tcp_client.c logger.h
udp_client.o: udp_client.c logger.h

# Normal compilation

normal: $(TARGET)

client: client.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o client
server: server.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o server

clean:
	$(RM) $(TARGET) $(obj) vgcore.*
	