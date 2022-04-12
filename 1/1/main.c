/*
Problem inf-III-01-1: posix/pipe/connect-2-processes
Программе передаётся два аргумента: CMD1 и CMD2. Необходимо запустить два процесса, выполняющих эти команды, и перенаправить стандартный поток вывода CMD1 на стандартный поток ввода CMD2.

В командной строке это эквивалентно CMD1 | CMD2.

Родительский процесс должен завершаться самым последним!
*/
#include <stdio.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char const *argv[]) {
  int pipes[2];
  pipe(pipes); // pipes[0] read pipes[1] write

  int stat1 = fork();
  if (stat1 == 0) {
    close(pipes[1]);
    dup2(pipes[0],0);
    execlp(argv[2], argv[2], NULL);
  }
  else{
    int stat2 = fork();
    if (stat2 == 0) {
      dup2(pipes[1],1);
      execlp(argv[1], argv[1], NULL);
    }
    else {
  //    close(pipes[1]);
      wait(&stat1);
      wait(&stat2);
    }
  }
  return 0;
}
