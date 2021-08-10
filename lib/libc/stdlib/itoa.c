#include "../../include/stdlib.h"
#include "../../include/ctypes.h"
#include "../../include/assert.h"

char* itoa(int val, int base){

    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)
{
        buf[i] = "0123456789abcdef"[val % base];
}
    return &buf[i+1];

}
char *itoas(int val, char *str, int base){
  int i = 30;
  for(; val && i ; --i, val /= base)
{
      str[i] = "0123456789abcdef"[val % base];
}
  return str;
}


inline char* litoa(long val, int base){

    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)
{
        buf[i] = "0123456789abcdef"[val % base];
}
    return &buf[i+1];

}

char *litoas(long val, char *str, int base){
  int i = 30;
  for(; val && i ; --i, val /= base)
{
      str[i] = "0123456789abcdef"[val % base];
}

  return str;
}
 char* uitoa(unsigned int val, int base){

  static char buf[32] = {0};
    int i = 30;

    for(; val && i ; --i, val /= base)
{
        buf[i] = "0123456789abcdef"[val % base];
}

    return &buf[i+1];

}
char *uitoas(unsigned int val, char *str, int base){
  int i = 30;
  for(; val && i ; --i, val /= base)
{
      str[i] = "0123456789abcdef"[val % base];
}
  return str;
}

 int atoi(const char **s)
{
	int i=0;

	while (isdigit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}
