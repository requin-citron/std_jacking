CC=gcc
CFLAGS=-Wall


all: std_jacking.c
		$(CC) $(CFLAGS) std_jacking.c gdb_utils.c -o std_jacking 
