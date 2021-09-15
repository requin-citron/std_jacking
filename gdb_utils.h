#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

void popen_wr(int *, int *,const char *);
void exec_cmd(int *, int *, char* ,size_t, char*);
int gdb_hook(char *,char *);
void gdb_unhook(char *, int );
