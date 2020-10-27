#include <symbols.h>

#ifndef D_MSG
	#define D_MSG
	void msg(const char *ms)
	{
		typedef int func(const char *, ...);
		func *f = (func *)F_PRINTF; // printf
		f(ms);
	}
#endif