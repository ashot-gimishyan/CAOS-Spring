/*
Problem inf-II-06-0: posix/exec/exec-python
Программе на стандартный поток ввода задается некоторое арифметическое выражение в синтаксисе языка python3.

Необходимо вычислисть это выражение, и вывести результат.

Использовать дополнительные процессы запрещено.
*/

#include <stdio.h>
#include <unistd.h>

int main()
{
    char expression[4096];
    char print[4096];

    int i = -1;

    do {
        ++i;
        scanf("%c", &expression[i]);
    } while (expression[i] != '\n');

    snprintf(print, sizeof(print), "print(%s)", expression);
    execlp("python3", "python3", "-c", print, NULL);
}
