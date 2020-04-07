# Set the following to '0' to disable log messages:
LOGGER ?= 1

#Paths
SHARED=../shared-src/

TARGET=standalone
CC=gcc
CFLAGS += -g -Wall -I "$(SHARED)" -lm -fPIC -DLOGGER=$(LOGGER)

# Source C files
globalsrc= $(SHARED)ini_parser.c $(SHARED)next_token.c $(SHARED)msleep.c
standalonesrc= standalone.c
src=$(standalonesrc) $(globalsrc)

obj=$(src:.c=.o)

$(TARGET): $(obj)
	$(CC) $(CFLAGS) $(LDFLAGS) $(obj) -o $@

# Shared headers
shareheaders= $(SHARED)ini_parser.h $(SHARED)next_token.h $(SHARED)logger.h $(SHARED)msleep.h

# Individual dependencies --
standalone.o: standalone.c $(shareheaders)

next_token.o: $(SHARED)next_token.c $(SHARED)next_token.h
ini_parser.o: $(SHARED)ini_parser.c $(SHARED)ini_parser.h

clean:
	$(RM) $(TARGET) $(obj) vgcore.* $(SHARED)ini_parser.o $(SHARED)next_token.o received.ini recieved.ini