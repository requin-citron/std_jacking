#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "gdb_utils.h"

#define TEMPLATE_RANDOM 10

char flag=1;
char *pid;
int fd_backup;


/*
(gdb) call (int)open("/tmp/httkigieha_1",066)
$1 = 3
(gdb) call (int)Quitmp/httkigieha_1",066)
(gdb) call (int)fcntl(3,4,(int)fcntl(0,3))
$2 = 0
(gdb) call (int)dup(2)
$3 = 4
(gdb) call (int)close(1)
$4 = 0
(gdb) call (int)close(1)
$5 = -1
(gdb) call (int)dup2(3,1)
*/

char *random_choise(int fd){
    size_t len = sizeof(char)*TEMPLATE_RANDOM+11+6;// max len of int and /tmp
    char *ret = malloc(len);
    if(ret == NULL){
      fprintf(stderr,"%s\n", strerror(errno));
      exit(1);
    }
    memset(ret, 0, len);
    char tmp[11] = {0};
    for(size_t i = 0; i < TEMPLATE_RANDOM; i ++){
      tmp[i] = 0x61+(rand()%26);
    }
    snprintf(ret, len, "/tmp/%s_%d",tmp, fd);
    return ret;
}

void on_exit_sig(int sig_id){
  flag=0;
  gdb_unhook(pid, fd_backup);
}

int main(int argc, char const *argv[]) {
  if(argc != 2){
    fprintf(stderr, "Usage: %s pid\n", argv[0]);
    return 1;
  }
  if(getuid() != 0 && geteuid() != 0){
    fprintf(stderr, "You need root privileges to attach process");
    return 5;
  }
  srand(time(NULL));
  //onexit setup
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = on_exit_sig;

  if(sigaction(SIGINT, &sa, NULL) == -1 &&
  sigaction(SIGQUIT, &sa, NULL) == -1){
    fprintf(stderr,"%s\n", strerror(errno));
    return 6;
  }
  //SETUP
  pid = (char *)argv[1];
  char *fifo_name_list[1]; //peut etre une upgrade dans le futur
  fifo_name_list[0] = random_choise((int)1);
  mkfifo(fifo_name_list[0], 0);
  chmod(fifo_name_list[0], 0666);

  fd_backup = gdb_hook((char *)argv[1], fifo_name_list[0]);
  char path[120]={0};
  char buff[512];
  size_t len = 0;

  sprintf(path, "/proc/%s/fd/4", argv[1]);
  FILE *victime = fopen(path,"w+");
  int tube = open(fifo_name_list[0],066);
  //fprintf(stderr,"fd open %d path:%s\n", tube, fifo_name_list[0]);
  if(victime==NULL){
    fprintf(stderr,"%s\n", strerror(errno));
    return 2;
  }

  fcntl(tube, F_SETFL, fcntl(tube,F_GETFL, 0) | O_NONBLOCK);
  while (flag) {
    memset(buff,0, 512);
    len = read(tube, buff, 512);
    write(STDOUT_FILENO, buff, len);
    write(fileno(victime), buff, len);
  }
  //CLEANUP
  fclose(victime);
  close(tube);
  remove(fifo_name_list[0]);
  free(fifo_name_list[0]);
  return 0;
}
