#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#define MAP_SIZE 4096UL
#define BOOTKEY_ADDR_IN 0x48310038UL
#define BOOTKEY_ADDR_DIR 0x48310034UL
#define SYSLED4_ADDR_OUT 0x4905803CUL
#define SYSLED4_ADDR_DIR 0x49058034UL

// Returnere en void pointer med den virtuelle adresse, skal have memory adressen og fd om argument
void* virtualAddr(unsigned long addr, int fd);

int main()
{
    // Åbner char device:
    // O_RDWR = read and write
    // O_SYNC = blokerer IO
    int fd;
    if( (fd = open("/dev/mem", (O_RDWR | O_SYNC))) == (-1))  //
    {
        printf("Could not open /dev/mem! ERROR: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    void* virtualBootkeyIn = virtualAddr(BOOTKEY_ADDR_IN, fd);
    void* virtualBootkeyDir = virtualAddr(BOOTKEY_ADDR_DIR, fd);
    void* virtualSysled4Out = virtualAddr(SYSLED4_ADDR_OUT, fd);
    void* virtualSysled4Dir = virtualAddr(SYSLED4_ADDR_DIR, fd);

    if ( (virtualBootkeyIn == (void*) -1) ||
         (virtualBootkeyDir == (void*) -1) ||
         (virtualSysled4Out == (void*) -1) ||
         (virtualSysled4Dir == (void*) -1) )
    {
        printf("One or more virtual addresses are wrong.\n");
        munmap(virtualBootkeyIn, MAP_SIZE);
        munmap(virtualBootkeyDir, MAP_SIZE);
        munmap(virtualSysled4Out, MAP_SIZE);
        munmap(virtualSysled4Dir, MAP_SIZE);
        return EXIT_FAILURE;
    }

    // Sætter input og output for BOOT_KEY og SYS_LED4
    //virtualBootkeyDir |=
    *(unsigned long*)virtualSysled4Dir &= 0xffffffef;

    for(;;)
    {
        if( (*(unsigned long*) virtualBootkeyIn & (0x80UL)) > 7)
        {
            printf("BOOT_KEY pushed!\n");
            *(unsigned long*)virtualSysled4Out &= 0xffffffefUL;
        }
        else
        {
            printf("BOOT_KEY not pushed!\n");
            *(unsigned long*)virtualSysled4Out |= (0x10UL);
        }
    }

    // Unmap
    munmap(virtualBootkeyIn, MAP_SIZE);
    munmap(virtualBootkeyDir, MAP_SIZE);
    munmap(virtualSysled4Out, MAP_SIZE);
    munmap(virtualSysled4Dir, MAP_SIZE);
}

void* virtualAddr(unsigned long addr, int fd)
{
    // mmap:
    // addr = adresse til map, her = 0 (OS vælger for dig)
    // len = længden af data til map
    // port = protection/beskyttelse, read and write
    // flags = MAP_SHARED så memory er ændret
    // fd = file descriptor
    // offset = hvor du vil starte mapping fra
    void* mapBase = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (addr & ~(MAP_SIZE-1)));
    if( mapBase == (void*) -1)
    {
        printf("Could not map memory for: %lu! ERROR: %s\n", addr, strerror(errno));
        return (void*) -1;
    }

    return mapBase + (addr & (MAP_SIZE-1));
}
