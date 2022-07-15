/*
Problem inf-III-05-2: posix/threads/ping-pong-threads-socketpair
Программе передается единственный аргумент - начальное значение 0<N≤100.

Необходимо создать два потока:

Принимает число, уменьшает его значение на 3, выводит его на стандартный поток вывода, и передает другому потоку.
Принимает число, увеличивает его значение на 5, выводит его на стандартный поток вывода, и передает другому потоку.
Начально значение передается первому потоку.

Взаимодействие прекращается, когда значение будет равно 0, либо станет строго больше 100. Последний поток, который получит число 0 или число >100, должен вывести его на стандартный поток вывода, и завершить работу.

Для передачи чисел между потоками используйте пару сокетов (man socketpair).
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>

const int BIG_VALUE = 1000000;
const int INT = sizeof(int);

int CalcNewn(int n, char flag)
{
    if (2 == flag)
        return n += 5;
    return n -= 3;
}

int SetFlag(int n) { return n > 100 || n == 0; }

void Send(int fd, char flag)
{
    for (;;) {
        int n = 0;
        read(fd, &n, INT);
        if (BIG_VALUE == n) {
            break;
        } else {
            n = CalcNewn(n, flag);
            printf("%i\n", n);

            if (SetFlag(n)) {
                write(fd, &BIG_VALUE, INT);
                break;
            }
        }
        write(fd, &n, INT);
    }
}

void* func_one(void* arg) { Send(*(int*)arg, 1); }
void* func_two(void* arg) { Send(*(int*)arg, 2); }

void StartPingPong(int N)
{
    int socket_pair[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair);
    write(socket_pair[1], &N, sizeof(unsigned int));

    pthread_t first;
    pthread_t second;

    pthread_create(&first, NULL, func_one, &socket_pair[0]);
    pthread_create(&second, NULL, func_two, &socket_pair[1]);

    pthread_join(first, NULL);
    pthread_join(second, NULL);
}

int main(int argc, char* argv[])
{
    StartPingPong(atoi(argv[1]));
}
