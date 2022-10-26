/*
Problem inf-III-02-0: posix/signals/count-sigint
Программа при запуске сообщает на стандартный поток вывода свой PID, выталкивает буфер вывода с помощью fflush, после чего начинает обрабатывать поступающие сигналы.

При поступлении сигнала SIGTERM необходимо вывести на стандартный поток вывода целое число: количество ранее поступивших сигналов SIGINT и завершить свою работу.

Семантика повединия сигналов (Sys-V или BSD) считается не определенной.
*/
#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

bool flag = false;
unsigned int count = 0;

void analyze(int signal)
{
    if (signal == SIGINT) count++;
    if (signal == SIGTERM) flag = true;
}

int main()
{
    struct sigaction sig_act;
    sig_act.sa_handler = analyze;

    sigaction(SIGINT, &sig_act, NULL);
    sigaction(SIGTERM, &sig_act, NULL);

    printf("%i\n", getpid());
    fflush(stdout);

    while (flag == false) {}
    printf("%i\n", count);
}
