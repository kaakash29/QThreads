
#include "qthread.h"
#include "greatest.h"
#include <unistd.h>
#include <assert.h>

int data = 0;

void* reader(void *p)
{
    int fd = (int) p;
    int x;
    ssize_t s = qthread_read(fd, &x, sizeof x);
    assert(s == sizeof x);
    data = x;
    return NULL;
}

void* writer(void *p)
{
    int fd = (int) p;
    //qt/hread_yield();
    assert(data == 0);
    int data = 42;
    ssize_t s = qthread_write(fd, &data, sizeof data);
    assert(s == sizeof data);
    return NULL;
}

TEST read_block(void)
{
    int pipefd[2];
    int s = pipe(pipefd);

    qthread_t read_thread, write_thread;
    qthread_create(&read_thread, NULL, reader, (void*) pipefd[0]);
    qthread_create(&write_thread, NULL, writer, (void*) pipefd[1]);

    qthread_join(read_thread, NULL);
    qthread_join(write_thread, NULL);

    ASSERT(data == 42);
    PASS();
}

SUITE(suite)
{
    RUN_TEST(read_block);
}

GREATEST_MAIN_DEFS();

int main(int argc, char** argv)
{
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(suite);
    GREATEST_MAIN_END();
}

