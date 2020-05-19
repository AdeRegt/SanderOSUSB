#include "../status.h"
#include "../kernel.h"
#include "program.h"

static int kernel_program_load_elf(const char *buffer)
{
    unsigned long gamma = loadelf((void *)buffer);
    if (gamma == 0)
    {
        return -EIO;
    }

    cls();
    void *(*foo)() = (void *)gamma;
    foo();
    
    return 0;
}

int kernel_program_load(const char *path)
{
    unsigned char *buffer = (unsigned char *)0x2000;
    if(fread((char *)path, buffer) == 0)
    {
        return -EIO;
    }
    
    if (iself(buffer))
    {
        kernel_program_load_elf((char *)buffer);
        return 0;
    }
    
    asm volatile("call 0x2000");
    return 0;
}