/*
Problem inf-IV-02-0: http/http-get
В аргументах командной строки передаются: 1) имя сервера; 2) путь к файлу на сервере, начинающися с символа /.

Необходимо выполнить HTTP-GET запрос к серверу вывести содержимое файла на стандартный поток вывода.

Запрещено использовать сторонние библиотеки.
*/

#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUF 256 * 4096

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

void HTTP_GET(int socket, char* server, char* path)
{
    char request[10000];
    request[0] = '\0';

    char* headers_end = "\r\n\r\n";
    sprintf(request + strlen(request), "GET %s HTTP/1.0\r\n", path);
    sprintf(request + strlen(request), "Host: %s%s", server, headers_end);

    write(socket, request, strlen(request));

    char line[10000],mock_response[BUF];
    mock_response[0] = '\0';
    int n = 0, first = 1;

    while ((n = read(socket, line, sizeof(line) - 1)) > 0) {
        line[n] = '\0';
        char* ptr = line;
        if (first) {
            ptr = strstr(line, headers_end) + 4;
        }
        first = 0;
        printf("%s", ptr);
    }
}

int main(int argc, char* argv[])
{
    if (argc == 3) {
        char ip[20];
        HostnameToIP(argv[1], ip);
        struct sockaddr_in addr = TCPSocketAddr(ip, 80);
        int server = Connect(addr);
        HTTP_GET(server, argv[1], argv[2]);
        return 0;
    }
    return 1;
}
