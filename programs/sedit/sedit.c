#include <symbols.h>
#define BUFFER_INIT_SIZE 0x1000
#define VK_UP 0xCB
#define VK_LEFT 0xCC
#define VK_RIGHT 0xCD
#define VK_DOWN 0xCE

unsigned char buffer[BUFFER_INIT_SIZE];
unsigned long filesizepointer = 0;
unsigned long cursorpos = 0;
char *filenamebuffer;

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

void clear()
{
	typedef void func();
	func *f = (func *)F_CLS; // putc
	f();
}

unsigned char getch()
{
	typedef unsigned char func();
	func *f = (func *)F_GETCH; // putc
	return f();
}

void msg(const char *ms)
{
	typedef int func(const char *, ...);
	func *f = (func *)F_PRINTF; // printf
	f(ms);
}

void setForeGroundBackGround(unsigned char fg, unsigned char bg)
{
	typedef void func(unsigned char, unsigned char);
	func *f = (func *)F_SETFOREGROUNDBACKGROUND; // putc
	f(fg, bg);
}

void putch(unsigned char c)
{
	typedef int func(const char);
	func *f = (func *)F_PUTC; // putc
	f(c);
}

char choose(char *message, int argcount, char **args)
{
	typedef char func(char *, int, char **);
	func *f = (func *)F_CHOOSE; // putc
	return f(message, argcount, args);
}

typedef struct
{
	int mouse_x;
	int mouse_y;
	int mouse_z;
	int mousePressed;
	int keyPressed;
} InputStatus;
InputStatus getInputStatus()
{
	typedef InputStatus func();
	func *f = (func *)F_GETINPUTSTATUS; // putc
	return f();
}

int main()
{
	for (int i = 0; i < BUFFER_INIT_SIZE; i++)
	{
		buffer[i] = 0;
	}
	char *c[3];
	c[0] = "new";
	c[1] = "open";
	c[2] = "exit";
	unsigned int t = choose("How can I help you", 3, c);
	if (t == 0)
	{
	}
	else if (t == 1)
	{
		typedef char *funct();
		funct *ft = (funct *)F_BROWSE; //browse
		filenamebuffer = ft();
		typedef char func(char *, unsigned char *);
		func *f = (func *)F_FREAD; // fread
		f(filenamebuffer, buffer);
		filesizepointer = 1;
		while (1)
		{
			if (buffer[filesizepointer++] == 0x00)
			{
				break;
			}
		}
	}
	else if (t == 2)
	{
		exit(0);
	}
	char textmode = 2; // 1 = overwrite , 2 = insert
	while (1)
	{
		clear();
		setForeGroundBackGround(0x02, 0x08);
		msg("type or press [end] for menu \n");
		msg("\n");
		setForeGroundBackGround(0x04, 0x01);
		for (unsigned long t = 0; t < (filesizepointer + 1); t++)
		{
			if (t == cursorpos)
			{
				setForeGroundBackGround(0x02, 0x08);
				putch('*');
				setForeGroundBackGround(0x04, 0x01);
			}
			else
			{
				putch(buffer[t]);
			}
		}

		volatile unsigned char d = getch();
		if (d == 1)
		{
			char *c[3];
			c[0] = "quit";
			c[1] = "save";
			if (textmode == 1)
			{
				c[2] = "insert";
			}
			else
			{
				c[2] = "overwrite";
			}
			c[3] = "cancel";
			unsigned int t = choose("Mainmenu", 4, c);
			if (t == 0)
			{
				exit(0);
			}
			else if (t == 1)
			{
				if (filenamebuffer == 0)
				{
					clear();
					msg("you should give up a file name first");
					getch();
					exit(0);
				}
				typedef char func(char *, unsigned char *, unsigned long);
				func *f = (func *)F_FWRITE; // fread
				f(filenamebuffer, (unsigned char *)buffer, (unsigned long)filesizepointer);
				clear();
				msg("the file is saved");
				getch();
			}
			else if (t == 2)
			{
				if (textmode == 1)
				{
					textmode = 2;
				}
				else
				{
					textmode = 1;
				}
			}
		}
		else if (d == VK_LEFT)
		{
			if (cursorpos)
			{
				cursorpos--;
			}
		}
		else if (d == VK_RIGHT)
		{
			if (cursorpos < filesizepointer)
			{
				cursorpos++;
			}
		}
		else if (d == 0x20)
		{
		}
		else
		{

			if (textmode == 1)
			{
				buffer[cursorpos] = d;
			}
			else if (textmode == 2)
			{
				for (unsigned int t = filesizepointer; t > (cursorpos - 1); t--)
				{
					buffer[t + 1] = buffer[t];
				}
				buffer[cursorpos] = d;
				cursorpos++;
				filesizepointer++;
			}
		}
	}
	for (;;)
		;
}