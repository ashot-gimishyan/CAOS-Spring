/*
Problem inf-III-07-0: posix/sockets/udp-client
Аргументом программы является целое число - номер порта на сервере localhost.

Программа читает со стандартного потока ввода целые числа в тектовом формате, и отправляет их в бинарном виде (little-endian) на сервер как UDP-сообщение.

В ответ сервер отправляет целое число (также в бинарном виде, little-endian), которое необходимо вывести на стандартный поток вывода.


*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = INADDR_ANY,
        .sin_port = htons(atoi(argv[1]))
    };

    int value;
    while (true) {
        if (scanf("%d", &value) > 0) {
            sendto(sock, &value, sizeof(value), 0, (struct sockaddr*)&addr, sizeof(addr));
            recv(sock, &value, sizeof(value), 0);
            printf("%d\n", value);
        } else break;
    }
}
