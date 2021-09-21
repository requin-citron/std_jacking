#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define STDOUT_HIJACK 1
#define STDERR_HIJACK 2

void popen_wr(int *, int *,const char *);
void exec_cmd(int *, int *, char* ,size_t, char*);
int gdb_hook(char *,char **,int );
void gdb_unhook(char *, int,int );
