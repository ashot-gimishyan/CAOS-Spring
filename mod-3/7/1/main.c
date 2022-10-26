/*
Problem inf-III-07-1: posix/sockets/icmp-ping
Программа принимает три аргумента: строку с IPv4-адресом, и два неотрицательных целых числа, первое из которых определяет общее время работы программы timeout, а второе - время между отдельными запросами в микросекундах interval.

Необходимо реализовать упрощённый аналог утилиты ping, которая определяет доступность удаленного хоста, используя протокол ICMP.

Программа должна последовательно отправлять echo-запросы к указанному адресу и подсчитывать количество успешных ответов. Между запросами, во избежание большой нагрузки на сеть, необходимо выдерживать паузу в interval микросекунд (для этого можно использовать функцию usleep).

Через timeout секунд необходимо завершить работу, и вывести на стандартный поток вывода количество полученных ICMP-ответов, соответствующих запросам.

В качестве аналога можно посмотреть утилиту /usr/bin/ping.

Указания: используйте инструменты ping и wireshark для того, чтобы исследовать формат запросов и ответов. Для того, чтобы выполняемый файл мог без прав администратора взаимодействовать с сетевым интерфейсом, нужно после компиляции установить ему capabilities командой: setcap cat_net_raw+eip PROGRAM. Контрольная сумма для ICMP-заголовков вычисляется по алгоритму из RFC-1071.
*/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

typedef struct {
    uint8_t type;
    uint8_t code;
    uint32_t checksum;
    uint8_t data[4];
} __attribute__((__packed__)) icmp_packet_t;

uint8_t flag = 0;
void alrm_handler(int signum) { if (SIGALRM == signum) { flag = 1; } }

static ushort checksum(void* in, int count)
{
    ushort* addr = in;
    ushort checksum;
    /* Compute Internet Checksum for "count" bytes
       *         beginning at location "addr".
       */

    // The register specifier asks the compiler to store
    // the variable in a way that allows for the fastest possible access.
    register long sum = 0;
    while (count > 1) {
        /*  This is the inner loop */
        sum += *(unsigned short*)addr++;
        count -= 2;
    }
    /*  Add left-over byte, if any */
    if (count > 0)
        sum += *(unsigned char*)addr;
    /*  Fold 32-bit sum to 16 bits */
    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);
    checksum = ~sum;
    return checksum;
}

// Address of the recipient: we send him a package
struct sockaddr_in Receiver(const char * ipv4_old) {
  struct in_addr ipv4_addr;
  inet_aton(ipv4_old, &ipv4_addr);

  struct sockaddr_in destination = {
    .sin_family = AF_INET,
    .sin_addr = ipv4_addr
  };
  return destination;
}

uint8_t Ping(char* ipv4)
{
    // AF_INET - ipv4 layer abstraction
    // SOCK_RAW - write the data ourselves, tcp/udp is not used
    // IPPROTO_ICMP - 1 is specified in the protocol field in the ipv4 packet (icmp)
    int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    icmp_packet_t req_icmp;
    req_icmp.code = 0;
    req_icmp.type = 8;
    for (int i = 0; i < 4; ++i) req_icmp.data[i] = 1;
    req_icmp.checksum = 0;
    req_icmp.checksum = checksum(&req_icmp, sizeof(icmp_packet_t));

    struct sockaddr_in destination = Receiver(ipv4);
    socklen_t socketlen = sizeof(struct sockaddr_in);
    sendto(sock_fd, &req_icmp, sizeof(req_icmp), 0, (struct sockaddr*)&destination, socketlen);

    icmp_packet_t answer;

    if (recvfrom(sock_fd, &answer, sizeof(answer), 0, (struct sockaddr*)&destination, &socketlen) <= 0) {
        return 0;
    }
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc == 4) {
        struct sigaction act;
        act.sa_handler = alrm_handler;
        sigaction(SIGALRM, &act, NULL);
        unsigned int sum_success = 0;
        alarm(atoi(argv[2])); // timeout
        while (!flag) {
            sum_success += Ping(argv[1]);
            usleep(atoi(argv[3])); // interval
        }
        printf("%i\n", sum_success);
        return 0;
    }
    printf("%s\n", "Error");
    return 1;
}
