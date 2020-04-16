# Set the following to '0' to disable log messages:
LOGGER ?= 0

TARGET=client
CC=gcc
CFLAGS += -g -Wall -I "../shared-src/" -lm -fPIC -DLOGGER=$(LOGGER)

# Source C files
globalsrc= ../shared-src/ini_parser.c ../shared-src/next_token.c ../shared-src/msleep.c
clientsrc= client.c tcp_client.c udp_client.c
src=$(clientsrc) $(globalsrc)

obj=$(src:.c=.o)

$(TARGET): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

# Shared headers
shareheaders= ../shared-src/ini_parser.h ../shared-src/next_token.h ../shared-src/logger.h ../shared-src/msleep.h

# Individual dependencies --
client.o: client.c tcp_client.h udp_client.h $(shareheaders)
tcp_client.o: tcp_client.c tcp_client.h $(shareheaders)
udp_client.o: udp_client.c udp_client.h $(shareheaders)

next_token.o: ../shared-src/next_token.c ../shared-src/next_token.h
ini_parser.o: ../shared-src/ini_parser.c ../shared-src/ini_parser.h

clean:
	$(RM) $(TARGET) $(obj) vgcore.* ../shared-src/ini_parser.o ../shared-src/next_token.o
