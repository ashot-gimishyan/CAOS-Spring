/*
Problem inf-IV-02-1: http/http-post
В аргументах командной строки передаются: 1) имя сервера; 2) путь к скрипту на сервере, начинающийся с символа /; 3) имя локального файла для отправки.

Необходимо выполнить HTTP-POST запрос к серверу, в котором отправить содержимое файла.

На стандартный поток ввода вывести ответ сервера (исключая заголовки).

Запрещено использовать сторонние библиотеки.

*/

#include <stdio.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <arpa/inet.h>

typedef struct stat Stat;
const int BUF_SIZE = 4096;

void HostnameToIP(const char* host, char* ip)
{
    struct in_addr** addr_list;
    struct hostent* host_resolved = gethostbyname(host);

    addr_list = (struct in_addr**)host_resolved->h_addr_list;
    if (host_resolved->h_length == 0) {
        return;
    }

    strcpy(ip, inet_ntoa(*addr_list[0]));
}

struct sockaddr_in TCPSocketAddr(char* ipv4_addr_str, unsigned int port)
{
    in_addr_t ip_addr = inet_addr(ipv4_addr_str);
    struct in_addr in_addr;
    in_addr.s_addr = ip_addr;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr = in_addr;
    addr.sin_port = htons(port);

    return addr;
}

int Connect(const struct sockaddr_in address)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == connect(sock, (const struct sockaddr*)&address, sizeof(address))) {
        return -1;
    }
    return sock;
}

char* MakeRequest(char* server_nm, char* path, int file_fd)
{
    Stat file_st; fstat(file_fd, &file_st);
    char* req = (char*)malloc(2 * BUF_SIZE); req[0] = '\0';

    sprintf(req + strlen(req), "POST %s HTTP/1.0\r\n", path);
    sprintf(req + strlen(req), "Host: %s\r\n", server_nm);
    sprintf(req + strlen(req), "Content-Length: %zu\r\n", file_st.st_size);
    sprintf(req + strlen(req), "\r\n"); // END

    return req;
}

void HTTP_POST(int socket, char* request, int file_fd)
{
    int n, done = 0;
    while ((n = write(socket, request + done, strlen(request) - done)) > 0) {
        done += n;
        if (done == strlen(request)) {
            break;
        }
    }

    char buf[BUF_SIZE];
    done = 0;
    while ((n = read(file_fd, buf, sizeof(buf))) > 0) {
        while ((n = write(socket, buf + done, n - done)) > 0) {
            done += n;
            if (done == n) {
                break;
            }
        }
    }

    int first = 1;
    char line[2 * BUF_SIZE];

    while ((n = read(socket, line, sizeof(line) - 1)) > 0) {
        line[n] = '\0';
        char* ptr = line;
        if (first && strncmp(line, "HTTP", 4) == 0) {
            ptr = strstr(line, "\r\n\r\n") + 4;
        }
        first = 0;
        printf("%s", ptr);
    }

    close(socket);
}

int main(int argc, char* argv[])
{
    if (argc == 4) {
        char ip[20];
        HostnameToIP(argv[1], ip); // server_nm
        struct sockaddr_in addr = TCPSocketAddr(ip, 80);

        int file_fd = open(argv[3], 0666);

        // argv[1] - server, argv[2] - sript, argv[3] - file
        char * Request = MakeRequest(argv[1], argv[2], file_fd);
        HTTP_POST(Connect(addr), Request, file_fd);

        return 0; // ok
    }
    return 1; // error
}
