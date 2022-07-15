/*
Problem inf-III-06-0: posix/threads/mutex
Программа запускается с двумя целочисленными аргументами: N>0 - количество итераций; и k>0 - количество потоков.

Необходимо создать массив из k вещественных чисел, после чего запустить k потоков, каждый из которых работает со своим элементом массива и двумя соседними.

Каждый поток N раз увеличивает значение своего элемента на 1, увеличивает значение соседа слева на 0.99, и увеличивает значение соседа справа на 1.01.

Для потоков, у которых нет соседей справа (k-1) или слева (0), соседними считать первое и последнее значение массива соответственно.

После того, как все потоки проведут N итераций, необходимо вывести значения всех элементов.

Запрещено использовать глобальные переменные.

Для вывода используйте формат %.10g.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    double* arr_doubles;
    pthread_mutex_t* mutex;
    unsigned int size, index, N;
} Arg;

void* func(void* tmp_arg)
{
    Arg* arg = (Arg*) tmp_arg;
    const int num = arg->N;

    int i = 0;
    while (i < num) {
        pthread_mutex_lock(arg->mutex);
        arg->arr_doubles[(arg->index + arg->size - 1) % arg->size] += .99;
        arg->arr_doubles[arg->index]++;
        arg->arr_doubles[(arg->index + arg->size + 1) % arg->size] += 1.01;
        pthread_mutex_unlock(arg->mutex);
        i++;
    }
}

int main(int argc, char** argv)
{

    const int K = atoi(argv[2]);
    double arr_doubles[K];
    pthread_t threads[K];

    int i;
    while (i < K) {
        arr_doubles[i] = 0;
        i++;
    }

    const int N = atoi(argv[1]);

    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    Arg args[K];
    int j = 0;
    while (j < K) {
        Arg tmp_arg = {.arr_doubles = arr_doubles, .index = j, .N = N, .size = K, .mutex = &mutex };
        args[j] = tmp_arg;
        pthread_create(&threads[j], NULL, func, &args[j]);
        j++;
    }

    int k = 0;
    do {
        pthread_join(threads[k], NULL);
        k++;
    } while (k < K);

    for (int i = 0; i < K; i++) {
        printf("%.10g\n", arr_doubles[i]);
    }
}
