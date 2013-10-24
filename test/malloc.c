#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
typedef unsigned long int uint32;
#define MEGA 1024*1024

int main(void) {
    char *buf;
    size_t buflen = 32 * MEGA;
    size_t r;
    int realloc_count;
    int fd;
    if( (fd = open("tmpfile", O_RDONLY)) == -1 ) return -1;

    realloc_count = 1;
    buf = malloc(buflen);
    while( (r = read(fd, buf + buflen * (realloc_count - 1), buflen)) > 0 ) {
printf("0x%lx\n", (uint32)buf);
        realloc_count++;
        buf = reallocf(buf, buflen * realloc_count);
        bzero(buf + buflen * (realloc_count - 1), buflen);
    }
    close(fd);

printf("0x%lx\n", (uint32)buf);
write(1, buf, buflen * (realloc_count - 1) + r);
write(1, "\n", 1);
    free(buf);
    return 0;
}
