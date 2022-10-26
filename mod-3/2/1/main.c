/*
Problem inf-III-02-1: posix/signals/do-actions
Программа при запуске сообщает на стандартный поток вывода свой PID, после чего читает со стандартного потока вывода целое число - начальное значение, которое затем будет изменяться.

При поступлении сигнала SIGUSR1 увеличить текущее значение на 1 и вывести его на стандартный поток вывода.

При поступлении сигнала SIGUSR2 - умножить текущее значение на -1 и вывести его на стандартный поток вывода.

При поступлении одного из сигналов SIGTERM или SIGINT необходимо завершить свою работу с кодом возврата 0.

Семантика повединия сигналов (Sys-V или BSD) считается не определенной.

Не забывайте выталкивать буфер вывода.
*/
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

sig_atomic_t number = 0;
sig_atomic_t flag = 0;

void handle_sa(int signum)
{
    if (SIGUSR1 == signum) {
        number += 1;
        printf("%i\n", number);
    } else if (SIGUSR2 == signum) {
        number *= -1;
        printf("%i\n", number);
    } else
        flag = 1;
}

int main()
{
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof sig_act);
    sig_act.sa_handler = handle_sa;

    sigaction(SIGINT, &sig_act, 0);
    sigaction(SIGTERM, &sig_act, 0);
    sigaction(SIGUSR1, &sig_act, 0);
    sigaction(SIGUSR2, &sig_act, 0);

    printf("%i\n", getpid());
    fflush(stdout);

    scanf("%i", &number);
    while (!flag) {
        pause();
    }
}
