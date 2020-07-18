// I always thought that I need to include headers where I defined the functions
// in the header but this made me realize that is not necesary.
// Is it better for compilation time tho?


unsigned long inportl (unsigned short _port){
    unsigned long rv;
    __asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportl (unsigned short _port, unsigned long _data){
    __asm__ __volatile__ ("outl %1, %0" : : "dN" (_port), "a" (_data));
}


unsigned short inportw (unsigned short _port){
    unsigned short rv;
    __asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportw (unsigned short _port, unsigned short _data){
    __asm__ __volatile__ ("outw %1, %0" : : "dN" (_port), "a" (_data));
}

unsigned char inportb (unsigned short _port){
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

void outportb (unsigned short _port, unsigned char _data){
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}
