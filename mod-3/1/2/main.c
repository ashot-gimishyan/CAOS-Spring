/*
Problem inf-III-01-2: posix/pipe/process-gcc-output-2
Программе в качестве аргумента передается имя файла программы на языке Си. Необходимо попытаться её скомпилировать с помощью штатного компилятора gcc, после чего вывести на стандартный поток вывода: количество строк программы с ошибками (error), и количество строк программы с предупреждениями (warning). В одной строке может быть найдено несколько ошибок или предупреждений, - нужно вывести именно количество строк.

Запрещено создавать временные файлы для сохранения вывода ошибок компилятора. Используйте передачу текста от компилятора через каналы.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    char* prog = argv[1];
    int prog_ln = (int)strlen(prog);

    int p_pair[2];
    pipe(p_pair);

    pid_t pid = fork();

    if (pid > 0) {
        close(p_pair[1]);
        FILE* error_gcc = fdopen(p_pair[0], "r");

        int error_ln = 0;
        int warning_ln = 0;

        size_t length;
        char* ln = NULL;

        int str_no, last_err_no = -1, last_warn_no = -1;

        while (getline(&ln, &length, error_gcc) != -1) {
            if (strncmp(ln, prog, prog_ln) != 0) {
                continue;
            }
            char* ln_ptr;
            str_no = (int)strtol(ln + prog_ln + 1, &ln_ptr, 10);
            if (str_no == 0) {
                continue;
            }
            strtol(ln_ptr + 1, &ln_ptr, 10);
            ln_ptr += 2;
            if (strncmp(ln_ptr, "error", 5) == 0) {
                error_ln += str_no != last_err_no;
                last_err_no = str_no;
            }
            if (strncmp(ln_ptr, "warning", 7) == 0) {
                warning_ln += str_no != last_warn_no;
                last_warn_no = str_no;
            }
        }
        printf("%i %i\n", error_ln, warning_ln);
    } else {
        close(p_pair[0]);
        dup2(p_pair[1], 2);
        dup2(p_pair[1], 1);
        execlp("gcc", "gcc", prog, NULL);
    }
}
