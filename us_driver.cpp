#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define BOOTKEY 0x48310034;
#define SYSLED4 0x49058034;
#define MAP_SIZE 2

int main()
{
    int fd;
    void *map_base;

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1)
        FATAL;
    printf("/dev/mem opened.\n");

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MAP_SIZE);


    if(munmap(map_base, MAP_SIZE) == -1) FATAL;
    close(fd);
    return 0;
}
