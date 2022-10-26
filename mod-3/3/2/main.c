/*
Problem int-III-03-2: posix/sockets/http-server-2
Необходимо реализовать программу-сервер, которой передаются два аргумента: номер порта и полный путь к каталогу с данными.

Программа должна прослушивать TCP-соединение на сервере localhost и указанным номером порта.

После получения сигнала SIGTERM или SIGINT сервер должен закончить обработку текущего соединения, если оно есть, после чего корректно завершить свою работу. Если при этом были запущены дочерние процессы - они должны быть завершены самим сервером.

Внимание: в этой задаче признаком конца строк считается пара символов "\r\n", а не одиночный символ '\n'.

Каждое соединение должно обрабатываться следующим образом:

Клиент отправляет строку вида GET ИМЯ_ФАЙЛА HTTP/1.1
Клиент отправляет произвольное количество непустых строк
Клиент отправляет пустую строку
После получения пустой строки от клиента, сервер должен отправить клиенту слеющие данные:

Строку HTTP/1.1 200 OK, если файл существует, или HTTP/1.1 404 Not Found, если файл не существует, или HTTP/1.1 403 Forbidden, если файл существует, но не доступен для чтения
Если файл не является выполняемым, то:

Строку Content-Length: %d, где %d - размер файла в байтах
Пустую строку
Содержимое файла as-is
Если файл является выполняемым, то после вывода строки HTTP/1.1 200 OK нужно выполнить этот файл, перенаправив его стандартный поток вывода клиенту as-is.

После отправки ответа клиенту, нужно закрыть соединение и не нужно ждать ожидать от клиента следующих запросов.


*/

#include <stdio.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>

const unsigned int SIZE = 4096;
sig_atomic_t flag = 0;

void s_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM || signum == SIGABRT)
        flag = 1;
}

void ReqRead(int fd_sock, char* req_txt)
{
    char buf[SIZE + 1];
    int current;
    int read_now = 0;
    while (0 < (current = recv(fd_sock, buf, SIZE, 0))) {
        buf[current] = '\0';
        strncpy(req_txt + read_now, buf, current);
        read_now += current;
        if (read_now >= 4 && strncmp(req_txt + read_now - 4, "\r\n\r\n", 4) == 0) {
            break;
        }
    }
    req_txt[read_now] = '\0';
}

int TryExecFile(char* current_path)
{
    int pipe_pair[2];
    pipe(pipe_pair);
    pid_t pid = fork();
    if (pid == 0) {
        dup2(pipe_pair[1], 1);
        execl(current_path, current_path, NULL, NULL);
        return -1; // error
    }
    close(pipe_pair[1]);
    return pipe_pair[0];
}

void ReqReadAgain(char* req_buf, char* f_nm)
{
    char* token = strtok(req_buf, " ");
    if (strncmp("GET", token, 3) != 0) {
        return;
    }
    if (token == NULL) {
        return;
    }

    char* ptrto_fname = strtok(NULL, " ");
    if (strlen(ptrto_fname) <= 1) {
        return;
    }
    if (ptrto_fname == NULL) {
        return;
    }
    strcpy(f_nm, ptrto_fname);
}

int MyWrite(int fd, char* text, int len)
{
    int current;
    int flag = 0;
    int written_now = 0;
    while (0 < (current = write(fd, text + written_now, len - written_now))) {
        written_now += current;
        if (written_now == len) {
            flag = 1;
            break;
        }
    }

    if (current == 0 || flag == 1) {
        return len;
    }
    return -1;
}

int FileCpy(int to, int from)
{
    int read_bytes = 0;
    char BUF[SIZE];
    int now_r;
    while ((now_r = read(from, BUF, SIZE)) > 0) {
        read_bytes += now_r;
        if (MyWrite(to, BUF, now_r) == -1) {
            return -1;
        }
    }
    if (now_r == 0) {
        return 0;
    }
    return -1;
}

void TryToAnswer(char* given_path, char* f_nm, int ans_fd)
{
    MyWrite(ans_fd, "HTTP/1.1", strlen("HTTP/1.1"));
    f_nm[0] == '/' ? ++f_nm : 0;

    char file[SIZE];
    sprintf(file, "%s/%s", given_path, f_nm);

    struct stat st;
    int status = stat(file, &st);
    if (status == -1) {
        ENOENT == errno ? MyWrite(ans_fd, " 404 Not Found\r\n", strlen(" 404 Not Found\r\n")) : MyWrite(ans_fd, " -1 Error\r\n", strlen(" -1 Error\r\n"));
        ;
        return;
    }

    if (!(S_IRUSR & st.st_mode)) {
        MyWrite(ans_fd, " 403 Forbidden\r\n", strlen(" 403 Forbidden\r\n"));
        return;
    }

    MyWrite(ans_fd, " 200 OK\r\n", strlen(" 200 OK\r\n"));
    if (S_IXUSR & st.st_mode) {
        int tmp_fd = TryExecFile(file);
        if (tmp_fd < 0) {
            return;
        }

        MyWrite(ans_fd, "\r\n", strlen("\r\n"));
        FileCpy(ans_fd, tmp_fd);
        close(tmp_fd);
    } else {
        MyWrite(ans_fd, "Content-Length: ", strlen("Content-Length: "));

        char sz_str[32];
        int sz_of_file = st.st_size;
        sprintf(sz_str, "%d", sz_of_file);

        MyWrite(ans_fd, sz_str, strlen(sz_str));
        MyWrite(ans_fd, "\r\n\r\n", strlen("\r\n\r\n"));

        int file_desc = open(file, O_RDONLY);
        FileCpy(ans_fd, file_desc);
    }
}

void RunServer(int fd_from_serv, char* given_path)
{
    int new_socket_fd;
    char f_nm[SIZE];
    char req_txt[SIZE];

    while (!flag) {
        struct sockaddr_in new_addr; // client
        socklen_t length;
        int new_socket_fd = accept(fd_from_serv, (struct sockaddr*)&new_addr, &length);
        if (-1 == new_socket_fd) {
            continue;
        }

        ReqRead(new_socket_fd, req_txt);
        ReqReadAgain(req_txt, f_nm);
        TryToAnswer(given_path, f_nm, new_socket_fd);

        shutdown(new_socket_fd, SHUT_RDWR);
        close(new_socket_fd);
    }
    printf("%s\n", "Finish");
}

int main(int argc, char* argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = s_handler;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);

    int port = atoi(argv[1]);

    struct in_addr addr_in;
    in_addr_t ip_addr = inet_addr("0.0.0.0");
    addr_in.s_addr = ip_addr;

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr = addr_in;

    int fd_from_serv = socket(AF_INET, SOCK_STREAM, 0);
    bind(fd_from_serv, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
    listen(fd_from_serv, SIZE);

    char* given_path = argv[2];
    RunServer(fd_from_serv, given_path);

    shutdown(fd_from_serv, SHUT_RDWR);
    close(fd_from_serv);
}
