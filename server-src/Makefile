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

# Individual dependencies --
server.o: server.c tcp_server.h udp_server.h logger.h next_token.h ini_parser.h
tcp_server.o: tcp_server.c tcp_server.h logger.h next_token.h ini_parser.h
udp_server.o: udp_server.c udp_server.h logger.h

next_token.o: next_token.c next_token.h
ini_parser.o: ini_parser.c ini_parser.h

clean:
	$(RM) $(TARGET) $(obj) vgcore.*