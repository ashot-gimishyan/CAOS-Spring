/*
Problem inf-III-04-1: highload/epoll-read-write-socket
Программе задается единственный аргумент - номер TCP-порта.

Необходимо принимать входящие соединения на TCP/IPv4 для сервера localhost, читать данные от клиентов в текстовом виде, и отправлять их обратно в текстовом виде, заменяя все строчные буквы на заглавные. Все обрабатываемые символы - из кодировки ASCII.

Одновременных подключений может быть много. Использовать несколько потоков или процессов запрещено.

Сервер должен корректно завершать работу при получении сигнала SIGTERM.

Указание: используйте неблокирующий ввод-вывод.
*/

#include <stdio.h>
#include <resolv.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

const int SIZE = 4096;
sig_atomic_t flag = 0;

void sig_handler(int signum) {
    if (signum == SIGINT || signum == SIGTERM || signum == SIGABRT) {
        flag = 1;
        exit(0);
    }
}

struct cli_unit {
    char buffer[4096];
    int buf_size;
    int fd;
};

int Read(struct cli_unit* cli)
{
    int fd = cli->fd;
    int by_read = 0, i = 0;
    if (0 < (by_read = read(fd, cli->buffer, SIZE))) {
        while (i < by_read) {
            cli->buffer[i] = (char)toupper(cli->buffer[i]);
            i++;
        }
        cli->buf_size = by_read;
    }
    return by_read;
}

int Write(struct cli_unit* cli)
{
    int wrote = 0;
    if (0 < cli->buf_size) {
        if (-1 == (wrote = write(cli->fd, cli->buffer, cli->buf_size))) {
            return -1;
        }
        cli->buf_size -= wrote;
        return cli->buf_size;
    }
    return 0;
}

struct cli_unit* InitConnect(int fd)
{
    struct cli_unit* cli = calloc(1, sizeof(struct cli_unit));
    cli->buf_size = 0;
    cli->fd = fd;
    return cli;
}

void Run(int server)
{
    struct epoll_event event;
    event.data.ptr = InitConnect(server);
    event.events = EPOLLIN;

    int desc_epoll = epoll_create1(0);
    epoll_ctl(desc_epoll, EPOLL_CTL_ADD, server, &event);

    sigset_t sig_set;
    sigfillset(&sig_set);
    sigdelset(&sig_set, SIGTERM);
    sigdelset(&sig_set, SIGINT);

    struct epoll_event events[SIZE];
    while (1) {
        int i = 0;
        int N_fd = 0;
        if (-1 == (N_fd = epoll_pwait(desc_epoll, events, SIZE, -1, &sig_set))) {
            return;
        }
        while (i < N_fd) {
            struct sockaddr_in cli_addr;
            socklen_t cli_addr_length;
            struct cli_unit* connection = events[i].data.ptr;

            if (server == connection->fd) {
                int socket_c = accept(server,
                    (struct sockaddr*)&cli_addr, &cli_addr_length);

                int flags = fcntl(socket_c, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(socket_c, F_SETFL, flags);

                struct cli_unit* cli = InitConnect(socket_c);
                event.data.ptr = cli;
                event.events = EPOLLIN;
                epoll_ctl(desc_epoll, EPOLL_CTL_ADD, socket_c, &event);
            }
            else {
                if (events[i].events & EPOLLIN) {
                    int status = Read(connection);
                    if (status > 0) {
                        events[i].events |= EPOLLOUT;
                        epoll_ctl(desc_epoll, EPOLL_CTL_MOD, connection->fd, &events[i]);
                    }
                }
                if (events[i].events & EPOLLOUT) {
                    int status = Write(connection);
                    if (0 == connection->buf_size) {
                        events[i].events &= ~EPOLLOUT;
                        epoll_ctl(desc_epoll, EPOLL_CTL_MOD, connection->fd, &events[i]);
                    }
                }
            }
            i++;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc == 2) {
        struct sigaction sig_act;
        memset(&sig_act, 0, sizeof(sig_act));
        sig_act.sa_handler = sig_handler;
        sigaction(SIGINT, &sig_act, NULL);
        sigaction(SIGTERM, &sig_act, NULL);
        sigaction(SIGABRT, &sig_act, NULL);

        int port = atoi(argv[1]);
        in_addr_t ip_addr = inet_addr("127.0.0.1");
        struct in_addr in_addr;
        in_addr.s_addr = ip_addr;

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr = in_addr;
        addr.sin_port = htons(port);

        int server = socket(AF_INET, SOCK_STREAM, 0);
        bind(server, (struct sockaddr*)&addr, sizeof(addr));
        listen(server, SIZE);

        int flags = fcntl(server, F_GETFL);
        flag |= O_NONBLOCK;
        fcntl(server, F_SETFL, flags);

        Run(server);

        puts("CLOSE");
        close(server);
    }
    else {
        printf("%s\n", "argv[2] ???");
        exit(0);
    }
}
