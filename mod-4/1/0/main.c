/*
Problem inf-IV-01-0: posix/dl/simple-dlopen
Программе передается два аргумента: имя файла с библиотекой и имя функции из этой библиотеки.

Гарантируется, что функция имеет сигнатуру:

double function(double argument);
На стандартном потоке ввода подаются вещественные числа. Необходимо применить к ним эту функцию, и вывести полученные значения. Для однозначности вывода используйте формат %.3f.
*/
#include <stdio.h>
#include <dlfcn.h>

int main(int argc, char* argv[])
{
    double (*func)(double);
    void* lib = dlopen(argv[1], 0);

    func = dlsym(lib, argv[2]);

    char* errstr = dlerror();
    if (NULL != errstr)
    	printf ("An error occurred: (%s)\n", errstr);

    double number;
    while (scanf("%lf", &number) > 0) {
        printf("%.3f\n", func(number));
    }
}
