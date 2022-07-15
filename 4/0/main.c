/*Problem inf-III-04-0: highload/epoll-read-fds-vector
Реализуйте функцию с сигнатурой:

          extern size_t
          read_data_and_count(size_t N, int in[N])

которая читает данные из файловых дескрипторов in[X] для всех 0 ≤ X < N , и возвращает суммарное количество прочитанных байт из всех файловых дескрипторов.

Скорость операций ввода-вывода у файловых дескрипторов - случайная. Необходимо минимизировать суммарное астрономическое время чтения данных.

По окончании чтения необходимо закрыть все файловые дескрипторы.

Указание: используйте неблокирующий ввод-вывод. Для тестирования можно использовать socketpair.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

const int SIZE = 4096;

void AddEvent(struct epoll_event* event, int ep_fd, int in_fdi)
{
    fcntl(in_fdi, F_SETFL, fcntl(in_fdi, F_GETFL) | O_NONBLOCK);
    event->data.fd = in_fdi;
    event->events = EPOLLIN;
    epoll_ctl(ep_fd, EPOLL_CTL_ADD, in_fdi, event);
}

extern size_t read_data_and_count(size_t N, int in[N])
{
    int epoll_fd = epoll_create(1);
    struct epoll_event events[N];

    int i = 0;
    while (i < N) {
        AddEvent(&events[i], epoll_fd, in[i]);
        i++;
    }

    char buf[SIZE];
    size_t sum = 0,read_now = 0;
    for (size_t done = 0; done < N;) {
        size_t ready = epoll_wait(epoll_fd, events, N - done, -1);
        while (--ready != -1) {
            if (0 >= (read_now = read(events[ready].data.fd, buf, SIZE))) {
                close(events[ready].data.fd);
                done++;
            } else sum += read_now;
        }
    }
    close(epoll_fd);
    return sum;
}
