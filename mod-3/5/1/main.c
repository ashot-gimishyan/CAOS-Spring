/*
Problem inf-III-05-1: posix/threads/parallel-sum
Единственным аргументом программы является целое число N>1 - число потоков, которые нужно создать.

На стандартном потоке ввода задается последовательность целых чисел.

Реализуйте программу, которая запускает N потоков, каждый из которых читает числа со стандартного потока ввода, и вычисляет частичные суммы.

На стандартный поток вывода необходимо вывести итоговую сумму всех чисел.

Минимизируйте объем используемой памяти настолько, насколько это возможно. Обратите внимание на ограничение по памяти.
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void* Sum(void* arg)
{
    int number;
    if (1 == scanf("%i", &number)) {
        do {
            int* sum_cell = arg;
            *sum_cell += number;
        } while (1 == scanf("%i", &number));
    }
}

int main(int argc, char* argv[])
{
    const int N = atoi(argv[1]);

    pthread_t partials[N];
    for (size_t i = 0; i < N; ++i) {
        partials[i] = 0;
    }

    pthread_t threads[N];
    for (size_t i = 0; i < N; ++i) {
        pthread_create(&threads[i], NULL, Sum, partials + i);
    }

    long result = 0;
    for (size_t i = 0; i < N; ++i) {
        pthread_join(threads[i], NULL);
        result += partials[i];
    }
    printf("%li\n", result);
}
