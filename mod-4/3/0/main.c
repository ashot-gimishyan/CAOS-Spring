/*
Problem inf-IV-03-0: openssl/dgst-sha512
Программе на стандартный поток ввода передается последовательность байт.

Необходимо вычислить контрольную сумму SHA-512 и вывести это значение в hex-виде c префиксом 0x.

Используйте API OpenSSL/LibreSSL. Запуск сторонних команд через fork+exec запрещен.

Отправляйте только исходный файл Си-программы с решением.
*/
#include <stdio.h>
#include <unistd.h>
#include <openssl/sha.h>

int main() {

    SHA512_CTX c;
    SHA512_Init(&c);
    unsigned char buf[SHA512_DIGEST_LENGTH];

    ssize_t n = 0;
    while((n = read(0, buf, sizeof(buf))) > 0){
        SHA512_Update(&c, buf, n);
    }

    int i = 0;
    printf("0x");
    SHA512_Final(buf, &c);

    while ( i < SHA512_DIGEST_LENGTH) {
        printf("%02x", buf[i]);
        i++;
    }
}
