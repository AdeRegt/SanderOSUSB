#include <symbols.h>

#ifndef D_EXIT
	#define D_EXIT
	void exit(int status)
	{
		if (status == 0)
		{
			int mode = 1;
			__asm__ __volatile__(
				"int $0x80"
				: "+a"(mode));
		}
		for (;;)
			;
	}
#endif