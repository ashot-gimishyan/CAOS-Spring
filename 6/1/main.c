/*
Problem inf-III-06-1: posix/threads/condvar
Программа принимает три аргумента: два 64-битных числа A и B, и 32-битное число N.

Затем создается дополнительный поток, которые генерирует простые числа в диапазоне от A до B включительно, и сообщает об этом основному потоку, с которого началось выполнение функции main.

Главный поток выводит на стандартный поток вывода каждое полученное число и завершает свою работу после того, как получит N чисел.

Запрещено использовать глобальные переменные.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

bool IsPrime(int64_t num)
{
    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) {
            return false; // no
        }
    }
    return true; // yes
}

typedef struct {
    bool flag;
    int32_t N;
    int64_t num;
    int64_t stop;
    int64_t start;
    pthread_cond_t ok;
    pthread_mutex_t mutex;
} Arg;

void* func(void* tmp_arg)
{
    Arg* arg = (Arg*)tmp_arg;

    unsigned int count = 0;
    const int64_t Start = arg->start;
    const int64_t Stop = arg->stop;

    pthread_mutex_lock(&arg->mutex);
    int64_t i = Start;
    while (i <= Stop) {
        if (!IsPrime(i)) {
            i++;
            continue;
        }
        else {
            arg->flag = true;
            arg->num = i;
            pthread_cond_signal(&arg->ok);
            do {
                pthread_cond_wait(&arg->ok, &arg->mutex);
            } while (arg->flag);
            if (count++ == arg->N) {
                break;
            }
            i++;
        }
    }
}

int main(int argc, char* argv[])
{
    if (4 == argc) {
        int64_t start = atoi(argv[1]);
        int64_t stop = atoi(argv[2]);
        int32_t N = atoi(argv[3]);

        pthread_t thread_prime;
        Arg range = {.N = N, .start = start, .stop = stop,
                     .mutex = PTHREAD_MUTEX_INITIALIZER,
                     .ok = PTHREAD_COND_INITIALIZER
        };
        pthread_create(&thread_prime, 0, func, &range);

        pthread_mutex_lock(&range.mutex);
        int i = 0;
        while (i < N) {
            do {
                pthread_cond_wait(&range.ok, &range.mutex);
            } while (range.flag == false);
            printf("%ld\n", range.num);
            range.flag = false;
            pthread_cond_signal(&range.ok);
            i++;
        }
        pthread_mutex_unlock(&range.mutex);
        return 0;
    }
    return 1; // few or many arguments
}
