/*
Problem inf-III-06-2: posix/threads/atomic
Реализуйте Lock-Free односвязный список из элементов:

typedef struct Item {
  struct Item *next;
  int64_t      value;
} item_t;
Программа принимает в качестве аргумента значения N - количество потоков, и k - количество элементов, создаваемых каждым потоком.

Этих потоков может быть очень много - несколько десятков!

Каждый поток должен добавить в односвязный список k элементов со значениями от i*k до (i+1)*k, где i - это номер потока от 0 до N.

После завершения работы всех потоков выведите все значения полученного односвязного списка.

Запрещено использовать мьютексы, семафоры или активное ожидание.

Минимизируйте используемый объем памяти.

Для уменьшения времени простоя используйте sched_yield в конце каждой итерации.
*/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct pitem {
    struct pitem* next;
    int64_t value;
} item_t;

struct lst {
    int length;
    item_t* start;
    item_t* stop;
};

struct lst* lst = NULL;
int64_t k = 0;

void* func(void* arg)
{
    item_t* pitem = NULL;
    int64_t idx = (int64_t)arg;
    int64_t i = (int64_t)arg * k;

    while (i < k * (idx + 1)) {
        pitem = malloc(sizeof(item_t));
        pitem->next = NULL;
        pitem->value = i;

        item_t* stop = atomic_exchange(&lst->stop, pitem);

        if (NULL != stop) {
            stop->next = pitem;
        }
        else if (NULL == stop){
            lst->start = pitem;
        }
        atomic_fetch_add(&lst->length, 1);
        i++;
    }
}

int main(int argc, char* argv[])
{
    if (3 == argc) {
        int64_t i;
        int64_t N = atoi(argv[1]);
        k = atoi(argv[2]);

        lst = malloc(sizeof(struct lst));
        lst->length = 0;
        lst->start = NULL;
        lst->stop = NULL;

        i = 0;
        pthread_t threads[N];
        while (i < N) {
            pthread_create(&threads[i], NULL, func, (void*)i);
            i++;
        }

        i = 0;
        while (i < N) {
            pthread_join(threads[i], NULL);
            i++;
        }

        i = 0;
        item_t* pitem = lst->start;
        while (i < lst->length) {
            fprintf(stdout, "%ld\n", pitem->value);
            pitem = pitem->next;
            i++;
        }
        return 0;
    }
    return 1;
}
