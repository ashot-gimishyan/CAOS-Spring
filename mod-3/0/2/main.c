/*
Problem inf-II-06-2: posix/exec/exec-cgi
Программе на стандартном потоке ввода задается текст вида

GET /some_path/script.py?a=123&b=abrakadabra HTTP/1.1
Host: www.example.com

Обратите внимание на последнюю пустую строку.

Необходимо сформировать и вывести на стандартный поток вывода HTTP-ответ, который в случае успеха имеет вид:

HTTP/1.1 200 OK
[содержимое вывода скрипта]
Если указанный файл не существует, то необходимо вывести текст

HTTP/1.1 404 ERROR

Если файл существует, но не является выполняемым, то текст

HTTP/1.1 403 ERROR

У скрипта могут (но не обязаны) быть CGI-параметры, которые ему нужно передавать для обработки.

Необходимо реализовать поддержку только GET-запросов, но не POST.

Гарантируется, что входные данных содержат только синтаксически корректные запросы.

Необходимо реализовать поддержку следующих переменных окружения:

HTTP_HOST
QUERY_STRING
REQUEST_METHOD
SCRIPT_NAME

*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <zconf.h>

const int max_name = 10000;
const char* tempfile = "/tmp/144442212";

void MakeResponse(int response_code)
{
    char response_status[10] = "OK";
    if (response_code != 200) {
        strcpy(response_status, "ERROR\n");
    }
    printf("HTTP/1.1 %d %s\n", response_code, response_status);
    fflush(stdout);
}

int main()
{
    char REQUEST_METHOD[10];
    char SCRIPT_NAME[max_name];
    char QUERY_STRING[max_name];
    char HTTP_HOST[max_name];
    char full_path[max_name];
    SCRIPT_NAME[0] = '.';

    scanf("%s ", REQUEST_METHOD);
    scanf("%s ", full_path);
    sscanf(full_path, "%[^?]s", &SCRIPT_NAME[1]);
    int have_query =
        sscanf(full_path + strlen(SCRIPT_NAME), "%s", QUERY_STRING);
    if (have_query != 1) {
        QUERY_STRING[0] = '\0';
    }
    scanf("HTTP/1.1\n");
    char header_name[max_name];
    char header_value[max_name];
    while (scanf("%s ", header_name) == 1) {
        scanf("%s\n", header_value);
        if (strcmp(header_name, "Host:") == 0) {
            strcpy(HTTP_HOST, header_value);
        }
    }

    int fd = open(SCRIPT_NAME, O_RDONLY);
    close(fd);

    if (-1 == fd) {
        MakeResponse(404);
    } else if (access(SCRIPT_NAME, X_OK) == -1) {
        MakeResponse(403);
    } else {
        int fork_status = fork();
        if (fork_status == 0) {
            MakeResponse(200);
            char* env[5];
            for (int i = 0; i < 4; ++i) {
                env[i] = (char*)malloc(max_name);
            }
            env[4] = NULL;
            strcpy(env[0], "HTTP_HOST=");
            strcat(env[0], HTTP_HOST);
            strcpy(env[1], "QUERY_STRING=");
            strcat(env[1], QUERY_STRING);
            strcpy(env[2], "REQUEST_METHOD=");
            strcat(env[2], REQUEST_METHOD);
            strcpy(env[3], "SCRIPT_NAME=");
            strcat(env[3], SCRIPT_NAME + 1);
            char* argv[2];
            argv[1] = NULL;
            argv[0] = SCRIPT_NAME;
            execve(SCRIPT_NAME, argv, env);
        } else {
            waitpid(fork_status, NULL, 0);
        }
    }
}
