/*
Problem inf-IV-02-2: http/curl-client
В аргументе командной строки передается полный URL веб-страницы в формате HTML.

Необходимо загрузить эту страницу и вывести на стандартный поток вывода только заголовок страницы, заключенный между тегами <title> и </title>.

Используйте LibCURL. На сервер нужно отпарвить только исходный файл, который будет скомпилирован и слинкован с нужными опциями.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

typedef CURL* pCURL;
const int SIZE = 128 * 1024 * 1024;

struct buffer {
    char* data;
    int size;
};

int call_func(char* ptr, int chunk_sz, int sz, void* local_data)
{
    struct buffer* buf = (struct buffer*)local_data;
    memcpy(buf->data + buf->size, ptr, chunk_sz * sz);
    buf->size += chunk_sz * sz;
    return chunk_sz * sz;
}

void PrintHeader(char* data)
{
    char* start = strstr(data, "<title>");
    char* stop = strstr(data, "</title>"); *stop = '\0';
    puts(strstr(data, "<title>") + strlen("<title>"));
}

int main(int argc, char* argv[])
{
    if (argc == 2) {
        pCURL curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, call_func);

        struct buffer buffer = {.data = calloc(SIZE, 1) };
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        const char* web_site = argv[1];
        curl_easy_setopt(curl, CURLOPT_URL, web_site);

        curl_easy_perform(curl);
        PrintHeader(buffer.data);

        return 0; // ok
    }
    return 1; // error
}
