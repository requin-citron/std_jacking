CC=gcc
CFLAGS=-Wall
RM=rm


all: std_jacking.o gdb_utils.o
		$(CC) $(CFLAGS) $^ -o std_jacking 
std_jacking.o:std_jacking.c
	$(CC) $(CFLAGS) $^ -c 
gdb_utils.o:gdb_utils.c gdb_utils.h
	$(CC) $(CFLAGS) $< -c 
clean: clean_objet
	$(RM) std_jacking
clean_objet:
	$(RM) *.o
