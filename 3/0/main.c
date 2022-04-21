/*
Problem int-III-03-0: posix/sockets/tcp-client
Программе передаются два аргумента: строка с IPv4-адресом в стандартной десятичной записи (четыре числа, разделенные точкой), и номер порта.

Программа должна установить соединение с указанным сервером, после чего читать со стандартного потока ввода целые знаковые числа в текстовом формате, и отправлять их в бинарном виде на сервер. Порядок байт - Little Endian.

В ответ на каждое полученное число, сервер отправляет целое число (в таком же формате), и все эти числа необходимо вывести на стандартный поток вывода в текстовом виде.

Если сервер по своей инициативе закроет соединение, то нужно завершить работу с кодом возврата 0.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
    struct in_addr in_addr;
    in_addr.s_addr = inet_addr(argv[1]);

    struct sockaddr_in addr = {.sin_family = AF_INET,
                               .sin_addr = in_addr,
                               .sin_port = htons(atoi(argv[2]))
                              };

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0 == connect(sock, (struct sockaddr*)&addr, sizeof addr)) {
        int val = 0;
        if (scanf("%d", &val) > 0) {
            do {
                int got_value;
                int res = write(sock, &val, sizeof(int));
                if (res > 0) {
                    res = read(sock, &got_value, sizeof(int));
                    if (res <= 0)
                        break;
                } else break;
                printf("%i\n", got_value);
            } while (scanf("%i", &val) > 0);
        }
    } else {
        perror("Connection error");
        return 1;
    }
    return 0;
}
