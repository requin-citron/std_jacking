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
int flag_std;


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

//! create random named pipe
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
//! ctrl+c hook
/*!
  \param sig_id signal id
  unhook target
*/
void on_exit_sig(int sig_id){
  flag=0;
  gdb_unhook(pid, fd_backup, flag_std);
}
//! software usage
void print_usage(const char *magie){
  fprintf(stderr, "Usage %s -p [pid] -[e|o]\n",magie);
  fprintf(stderr, "   -e             stderr\n");
  fprintf(stderr, "   -o             stdout\n");
  exit(6);
}

int main(int argc, char const **argv) {
  char *p = NULL;
  int c;
  while((c = getopt(argc, (char * const *)argv, "oep:")) != -1){
    switch (c) {
      case 'o':
          flag_std |= STDOUT_HIJACK;
          break;
      case 'e':
          flag_std |= STDERR_HIJACK;
          break;
      case 'p':
          p = optarg;
          break;
      default:
          print_usage(argv[0]);
    }
  }
  if(p == NULL || flag_std == 0){
    print_usage(argv[0]);
  }
  if(getuid() != 0 && geteuid() != 0){
    fprintf(stderr,"you must be root to hijack process\n");
    return 5;
  }
  srand(time(NULL));
  //onexit setup
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = on_exit_sig;

  if(sigaction(SIGINT, &sa, NULL) == -1 ||
  sigaction(SIGQUIT, &sa, NULL) == -1){
    fprintf(stderr,"%s\n", strerror(errno));
    return 6;
  }
  //SETUP
  pid = (char *)p;
  char *fifo_name_list[1]; //peut etre une upgrade dans le futur
  fifo_name_list[0] = random_choise((int)(1));
  mkfifo(fifo_name_list[0], 0);
  chmod(fifo_name_list[0], 0666);

  fd_backup = gdb_hook((char *)pid, fifo_name_list, flag_std);
  char path[120]={0};
  char buff[512];
  size_t len = 0;

  sprintf(path, "/proc/%s/fd/%d", pid, fd_backup);
  FILE *victime = fopen(path,"w+");
  int tube = open(fifo_name_list[0],066);


  if(victime==NULL){
    fprintf(stderr,"%s\n", strerror(errno));
    return 2;
  }


  fcntl(tube, F_SETFL, fcntl(tube,F_GETFL, 0) | O_NONBLOCK);
  while (flag) {
    memset(buff,0, 512);
    //stdout
    len = read(tube, buff, 512);
    write(STDOUT_FILENO, buff, len);
    write(fileno(victime), buff, len);
    usleep(300); //to not take too much cpu time

  }
  //CLEANUP
  fclose(victime);
  close(tube);
  remove(fifo_name_list[0]);
  free(fifo_name_list[0]);
  return 0;
}
