# Set the following to '0' to disable log messages:
LOGGER ?= 0

#Paths
SHARED=../shared-src/

TARGET=standalone
CC=gcc
PCAP= -I /usr/include/pcap -lpcap
CFLAGS += -g -Wall -I "$(SHARED)" -lm -fPIC -DLOGGER=$(LOGGER)

# Source C files
globalsrc= $(SHARED)ini_parser.c $(SHARED)next_token.c $(SHARED)msleep.c
standalonesrc= standalone.c create_hdrs.c sniff_rst.c send_tcp.c
src=$(standalonesrc) $(globalsrc)

obj=$(src:.c=.o)

$(TARGET): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@ $(PCAP)

# Shared headers
shareheaders= $(SHARED)ini_parser.h $(SHARED)next_token.h $(SHARED)logger.h $(SHARED)msleep.h $(SHARED)structs.h

# Individual dependencies
standalone.o: standalone.c create_hdrs.h sniff_rst.h send_tcp.h $(shareheaders)
create_hdrs.o: create_hdrs.c create_hdrs.h $(shareheaders)
sniff_rst.o: sniff_rst.c sniff_rst.h $(shareheaders)
send_tcp.o: send_tcp.c send_tcp.h $(shareheaders)


next_token.o: $(SHARED)next_token.c $(SHARED)next_token.h
ini_parser.o: $(SHARED)ini_parser.c $(SHARED)ini_parser.h

clean:
	$(RM) $(TARGET) $(obj) vgcore.* $(SHARED)ini_parser.o $(SHARED)next_token.o