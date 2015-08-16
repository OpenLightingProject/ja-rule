CC=gcc

all: hex2dfu uid2dfu

hex2dfu: dfu.c hex2dfu.c utils.c
	$(CC) -o $@ $^

uid2dfu: dfu.c uid2dfu.c utils.c
	$(CC) -o $@ $^

.PHONY: clean

clean:
	rm hex2dfu uid2dfu
