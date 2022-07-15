/*
Problem inf-IV-03-1: openssl/decrypt-aes-256-cbc
Программе передается аргумент - пароль.

На стандартый поток ввода подаются данные, зашифрованные алгоритмом AES-256-CBC с солью. Для получения начального вектора и ключа из пароля и соли используется алгоритм SHA-256.

Необходимо расшифровать данные и вывести их на стандартый поток вывода.

Используйте API OpenSSL/LibreSSL. Запуск сторонних команд через fork+exec запрещен.

Отправляйте только исходный файл Си-программы с решением.


*/

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/evp.h>

const int STDIN = 0;
const int STDOUT = 1;
const int SIZE = 1024;

int rd_count = 0;
int size = SIZE * SIZE;

char* Decrypt_AES(int*, char*, EVP_CIPHER_CTX*);

int main(int argc, char** argv)
{
    if (argc != 2) {
        puts("Too few arguments.");
        return 1;
    }

    char* data = (char*)malloc(size);

    while (true) {
        int bytes_in = read(STDIN, data + rd_count, 4 * SIZE);
        if (bytes_in > 0) {
            rd_count += bytes_in;
            if (rd_count > size - 4 * SIZE) {
                char tmp[size];
                strcpy(tmp, data);
                size *= 2;
                data = realloc(data, size);
                strcpy(data, tmp);
            }
            continue;
        }
        break;
    }

    // Create a context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    char* salt = data + 8; // Salt size 8 bytes
    char key[SIZE], vect_init[SIZE];

    // Generation of key and initial vector from
    // arbitrary length password and 8-byte salt
    EVP_BytesToKey(
        EVP_aes_256_cbc(), // encryption algorithm
        EVP_sha256(), // password hashing algorithm
        salt, // salt
        argv[1], strlen(argv[1]), // password
        1, // hash iterations
        key, // result: key of the required length
        vect_init // result: initial vector of desired length
        );

    // Initial stage: initialization
    EVP_DecryptInit(
        ctx, // state storage context
        EVP_aes_256_cbc(), // encryption algorithm
        key, // key of the required length
        vect_init // initial value of the desired size
        );

    int i = 0;
    int num = rd_count - 16;
    int* data_size = &num;

    char* result = Decrypt_AES(data_size, data, ctx);

    while (i < *data_size) {
        printf("%c", result[i]);
        i++;
    }
}

char* Decrypt_AES(int* size, char* data_obr, EVP_CIPHER_CTX* ctx)
{
    int final_len = 0;
    int len = *size;
    char* data = data_obr + 16;
    char* res = (char*)malloc(len);

    EVP_DecryptInit_ex(ctx, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate(ctx, res, &len, data, *size);
    EVP_DecryptFinal_ex(ctx, res + len, &final_len);

    *size = len + final_len;
    return res;
}
