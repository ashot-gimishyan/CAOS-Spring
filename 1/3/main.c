/*
Problem inf-III-01-3: posix/pipe/connect-n-processes
Программе передаётся произвольное количество аргументов: CMD1, CMD2, ..., CMDN.

Необходимо реализовать эквивалент запуска их командной строки: CMD1 | CMD2 | ... | CMDN.

Родительский процесс должен завершаться самым последним!
*/

#include <wait.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct {
    char* prog;
    long in;
    int* rp;
} Params;

long Run(Params In)
{
    long output = In.rp[1];
    pid_t chld = fork();
    if (chld == -1)
        return 1;

    else if (!chld) {
        close(In.rp[0]);
        dup2(In.in, 0);
        dup2(output, 1);
        execlp(In.prog, In.prog, NULL);
    }

    wait(&chld);
    close(In.rp[1]);
    return 2;
}

int main(int argc, char* argv[])
{
    if (argc >= 2) {
        int rp[2];
        pipe(rp);

        long read = 0;
        long i = 1;

        Params param;
        while (i < argc) {
            if (argc == i + 1) {
                close(rp[1]);
                rp[1] = 1;
            }

            param.prog = argv[i];
            param.in = read;
            param.rp = rp;

            if (1 == Run(param)) {
                return 0;
            }
            else {
                read = rp[0];
                pipe(rp);
            }
            i++;
        }
    }
    return 1;
}
