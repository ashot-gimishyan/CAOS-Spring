/*
Problem inf-II-06-1: posix/exec/exec-gcc
Программе на стандартном потоке ввода задается выражение в синтаксисе языка Си.

Необходимо вычислить значение этого выражения (итоговый результат представим типом int) и вывести его на стандартный поток вывода.
*/

#include <stdio.h>
#include <unistd.h>
#include <wait.h>

int main()
{
    char expression[4096];
    int i = 0;
    do {
        scanf("%c", expression + i);
    } while (expression[i++] != '\n');
    expression[i] = '\0';

    char* temp_cfile = "out.c";
    FILE* open_cfile = fopen(temp_cfile, "w");
    fprintf(open_cfile, "#include <stdio.h>\nint main(){printf(\"%%d\\n\",(%s));}", expression);
    fclose(open_cfile);

    int status = fork();
    if (0 == status) {
        execlp("gcc", "gcc", temp_cfile, NULL);
    } else {
        wait(&status);
        execlp("./a.out", "./a.out", NULL);
    }
}
