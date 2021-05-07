// I always thought that I need to include headers where I defined the functions
// in the header but this made me realize that is not necesary.
// Is it better for compilation time tho?


unsigned long inportl (unsigned short _port){
    asm volatile ("cli");
    unsigned long rv;
    __asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    asm volatile ("sti");
    return rv;
}

void outportl (unsigned short _port, unsigned long _data){
    asm volatile ("cli");
    __asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
    asm volatile ("sti");
}


unsigned short inportw (unsigned short _port){
    asm volatile ("cli");
    unsigned short rv;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    asm volatile ("sti");
    return rv;
}

void outportw (unsigned short _port, unsigned short _data){
    asm volatile ("cli");
    __asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
    asm volatile ("sti");
}

unsigned char inportb (unsigned short _port){
    asm volatile ("cli");
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    asm volatile ("sti");
    return rv;
}

void outportb (unsigned short _port, unsigned char _data){
    asm volatile ("cli");
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
    asm volatile ("sti");
}
