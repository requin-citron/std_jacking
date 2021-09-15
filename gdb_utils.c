#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include "gdb_utils.h"

void popen_wr(int *pipe_stdin, int *pipe_stdout,const char *pid){
    int status = pipe(pipe_stdin);
    int status1 = pipe(pipe_stdout);
    if(status == -1 || status1 == -1){
      fprintf(stderr,"%s\n", strerror(errno));
      exit(2);
    }

    switch(fork()){
      case -1:
        fprintf(stderr, "%s\n", strerror(errno));
        exit(3);
        break;
      case 0: // child
        char *params[] = {"/usr/bin/gdb","--nx", "-q","-p",(char *)pid,NULL};
        //fprintf(stderr,"%s\n", params[0]);
        //fprintf(stderr,"%s\n", params[1]);
        fclose(stderr);
        dup2(pipe_stdin[0],STDIN_FILENO);
        dup2(pipe_stdout[1],STDOUT_FILENO);
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        execve("/usr/bin/gdb",params,NULL);
        fprintf(stderr,"%s\n", strerror(errno));
        exit(4);
        break;
    }

}

void exec_cmd(int *pipe_stdin, int *pipe_stdout,char *out,size_t len, char* cmd){

    write(pipe_stdin[1],cmd, strlen(cmd));
    if(strcmp(cmd,"quit\n")!=0 && strcmp(cmd,"detach\n")!=0){
      do {
        read(pipe_stdout[0], out, len);
      } while(strchr(out, '=') == NULL);
    }


}

int gdb_hook(char *pid,char **fifo_name){
  int pipe_stdin[2];
  int pipe_stdout[2];
  char cmd[512]={0};
  char out[512] = {0};
  size_t len = 511;
  popen_wr(pipe_stdin, pipe_stdout,pid);
  //open fifo file stdout
  snprintf(cmd, 511,"call (int)open(\"%s\", 066)\n",fifo_name[0]);
  exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);
  int fd_fifo_stdout = atoi(strchr(out,'=')+1);

  //open fifo file stderr
  //snprintf(cmd, 511,"call (int)open(\"%s\", 066)\n",fifo_name[1]);
  //exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);
  //int fd_fifo_stderr = atoi(strchr(out,'=')+1);

  //stage 2 make set same flag as stdout
  snprintf(cmd, 511,"call (int)fcntl(%d,4,(int)fcntl(1,3))\n",fd_fifo_stdout);
  exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);
  //make set same flag as stderr
  //snprintf(cmd, 511,"call (int)fcntl(%d,4,(int)fcntl(2,3))\n",fd_fifo_stderr);
  //exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);

  //stage3
  exec_cmd(pipe_stdin, pipe_stdout,out,len, "call (int)dup(1)\n");
  int aim_stdout_backup = atoi(strchr(out,'=')+1);
  //stage 4 close stdout
  exec_cmd(pipe_stdin, pipe_stdout,out,len, "call (int)close(1)\n");
  //stage4 close stderr
  exec_cmd(pipe_stdin, pipe_stdout,out,len, "call (int)close(2)\n");
  //stage 5 dup2 stdout
  snprintf(cmd, 511,"call (int)dup2(%d,1)\n",fd_fifo_stdout);
  exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);
  //stage 5 stderr
  snprintf(cmd, 511,"call (int)dup2(%d,2)\n",fd_fifo_stdout);
  exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);
  //clean and exit
  exec_cmd(pipe_stdin, pipe_stdout,out,len, "detach\n");
  exec_cmd(pipe_stdin, pipe_stdout,out,len, "quit\n");
  wait(NULL);
  return aim_stdout_backup;
}

void gdb_unhook(char *pid, int pts_fd){
  int pipe_stdin[2];
  int pipe_stdout[2];
  char cmd[512] = {0};
  char out[512] = {0};
  size_t len = 511;
  popen_wr(pipe_stdin, pipe_stdout,pid);
  //stdout
  fprintf(stderr, "%d\n", pts_fd);
  snprintf(cmd, len, "call (int)dup2(%d,1)\n", pts_fd);
  exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);
  //stderr
  snprintf(cmd, len, "call (int)dup2(%d,2)\n", pts_fd);
  exec_cmd(pipe_stdin, pipe_stdout,out,len, cmd);
  exec_cmd(pipe_stdin, pipe_stdout,out,len, "detach\n");
  exec_cmd(pipe_stdin, pipe_stdout,out,len, "quit\n");
  wait(NULL);
}
