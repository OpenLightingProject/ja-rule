CC=gcc
CFLAGS=-std=c99

all: hex2dfu uid2dfu

hex2dfu: dfu.c hex2dfu.c utils.c
	$(CC) $(CFLAGS) -o $@ $^

uid2dfu: dfu.c uid2dfu.c utils.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm hex2dfu uid2dfu
