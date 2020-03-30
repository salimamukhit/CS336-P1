# Set the following to '0' to disable log messages:
LOGGER ?= 1

TARGET=client
CC=gcc
CFLAGS += -g -Wall -lm -fPIC -DLOGGER=$(LOGGER)

# Source C files
globalsrc= ini_parser.c next_token.c
clientsrc= client.c tcp_client.c udp_client.c
src=$(clientsrc) $(globalsrc)

obj=$(src:.c=.o)

$(TARGET): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

# Individual dependencies --
client.o: client.c tcp_client.h udp_client.h logger.h next_token.h ini_parser.h
tcp_client.o: tcp_client.c tcp_client.h logger.h
udp_client.o: udp_client.c udp_client.h logger.h

next_token.o: next_token.c next_token.h
ini_parser.o: ini_parser.c ini_parser.h

clean:
	$(RM) $(TARGET) $(obj) vgcore.*
