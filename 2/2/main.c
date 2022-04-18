/*
Программе в качестве аргументов передаются N имен текстовых файлов.

Программа должна обрабатывать множество сигналов от SIGRTMIN до SIGRTMAX, причем номер сигнала в диапазоне от SIGRTMIN+1 определяет порядковый номер файла из аргументов:

x = signo - SIGRTMIN; // SIGRTMIN <= signo <= SIGRTMAX
                      // 1 <= x <= SIGRTMAX-SIGRTMIN
При получении очередного сигнала необходимо прочитать одну строку из определенного файла и вывести её на стандартный поток вывода.

При получении сигнала с номером SIGRTMIN, т.е. при номере аргумента, равным 0, - корректно завершить свою работу с кодом 0.

Все остальные сигналы нужно игнорировать.

Если для вывода используются высокоуровневые функции стандартной библиотеки Си, то необходимо выталкивать буфер обмена после вывода каждой строки.
*/
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int flag_of_exit = 0;
FILE* arr_of_files[4096];

void handle_sa(int signum)
{
    int num = signum - SIGRTMIN;
    size_t line_length = 0;
    FILE* file_ptr = arr_of_files[num];

    if (0 == num) { // SIGRTMIN == signum
        flag_of_exit = 1;
    }

    if (flag_of_exit != 1) {
        char* tmp_line = NULL;
        getline(&tmp_line, &line_length, file_ptr);
        printf("%s", tmp_line);
        fflush(stdout);
    } else {
        return;
    }
}

int main(int argc, char* argv[])
{
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof sig_act);
    sig_act.sa_handler = handle_sa;
    sigaction(SIGINT, &sig_act, 0);

    int i = SIGRTMIN;
    while (i <= SIGRTMAX) {
        sigaction(i, &sig_act, 0);
        ++i;
    }

    int j = 1;
    while (j < argc) {
        arr_of_files[j] = fopen(argv[j], "r");
        ++j;
    }

    while (!flag_of_exit) {
        pause();
    }
}
