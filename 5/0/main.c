/*
Problem inf-III-05-0: posix/threads/simple-create-join
На стандартном потоке ввода задается последовательность целых чисел.

Необходимо прочитать эти числа, и вывести их в обратном порядке.

Каждый поток может прочитать или вывести только одно число.

Используйте многопоточность, запуск процессов запрещен.
*/
#include <stdio.h>
#include <pthread.h>

void* Reader(void* arg)
{
    int num;
    if (1 == scanf("%i", &num)) {
        pthread_t thread;
        pthread_create(&thread, NULL, Reader, NULL);
        pthread_join(thread, NULL);
        printf("%i\n", num);
    }
}

int main()
{
    Reader(NULL);
}
