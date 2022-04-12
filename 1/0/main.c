/*
Problem inf-III-01-0: posix/pipe/launch
Реализуйте программу, которая принимает два аргумента: CMD - команда для запуска, IN - имя файла, направляемого на ввод.

Программа должна запускать указанную команду, передавая ей на вход содежимое файла IN.

На стандартный поток вывода вывести количество байт, которое было записано запущенной командой в стандартный поток вывода. Вывод самой команды никуда выводить не нужно.
*/
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

const int BUF_SIZE = 4096;
int main(int argc, char* argv[])
{
    int fds_pair[2];
    pipe(fds_pair);
    char buf[BUF_SIZE];
    int tmp_count=0, all_bytes = 0;

    if ( 0 == fork() ) {
        dup2(fds_pair[1], 1);
        execlp(argv[1], argv[1], argv[2], NULL); // CMD IN
    } else {
        close(fds_pair[1]);
        do {
            all_bytes += tmp_count;
            tmp_count = read(fds_pair[0], buf, BUF_SIZE);
        } while (tmp_count > 0);
        printf("%i\n", all_bytes);
    }
}
