# Set the following to '0' to disable log messages:
LOGGER ?= 1

TARGET=server
CC=gcc
CFLAGS += -g -Wall -lm -fPIC -DLOGGER=$(LOGGER)

# Source C files
globalsrc= ini_parser.c next_token.c
serversrc= server.c tcp_server.c udp_server.c
src=$(serversrc) $(globalsrc)

obj=$(src:.c=.o)

$(TARGET): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

# Shared headers
shareheaders= ../shared-src/ini_parser.h ../shared-src/next_token.h ../shared-src/logger.h

# Individual dependencies --
server.o: server.c tcp_server.h udp_server.h $(shareheaders)
tcp_server.o: tcp_server.c tcp_server.h $(shareheaders)
udp_server.o: udp_server.c udp_server.h $(shareheaders)

next_token.o: ../shared-src/next_token.c ../shared-src/next_token.h
ini_parser.o: ../shared-src/ini_parser.c ../shared-src/ini_parser.h

clean:
	$(RM) $(TARGET) $(obj) vgcore.*