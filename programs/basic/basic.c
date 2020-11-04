#include <symbols.h>
#include <stdio.h>
#include <stdlib.h>
#include <SanderOSUSB.h>

#define BUFFER_INIT_SIZE 0x1000
#define MAX_SIZE_STRING 128
#define MAX_SIZE_NEST 10
#define MAX_NATIVE_FUNCTION 10

#define TOK_IS_EMPTY 0
#define TOK_IS_KEYWORD 1
#define TOK_IS_STRING 2
#define TOK_IS_VARIABLE 3
#define TOK_IS_LABEL 4

typedef struct {
	char *name;
	int length;
	unsigned long location;
}NativeFunction;

typedef struct{
	char definition[MAX_SIZE_STRING+2];
	char type;
}Token;

char buffer[BUFFER_INIT_SIZE];
unsigned long filesizepointer = 0;
unsigned long cursorpos = 0;
char *filenamebuffer;

unsigned long integer_variables['Z'-'A'];
unsigned char string_variables[('8'-'1')*MAX_SIZE_STRING];
unsigned long nests[MAX_SIZE_NEST];
int nestpointer = 0;

unsigned linenumber = 0;
NativeFunction functiontable[MAX_NATIVE_FUNCTION];
int functiontablepointer = 0;

void error(const char *ms)
{
	typedef int func(const char *, ...);
	func *f = (func *)F_PRINTF; // printf
	f("\nAn error occured at line %x %s \n",linenumber,ms);
	for(;;);
}

void set_integer_variable(unsigned char varname,unsigned long value){
	integer_variables[varname-'A'] = value;
}

void set_string_variable(unsigned char varname,unsigned char message[MAX_SIZE_STRING]){
	int offset_string = varname - '1';
	for(int i = 0 ; i < MAX_SIZE_STRING ; i++){
		string_variables[offset_string+i] = message[i];
	}
}

void clear_variables(){

	//
	// clear all integer variables
	for(unsigned char i = 'A' ; i < 'Z' ; i++){
		set_integer_variable(i,0);
	}

	//
	// clear all string variables
	unsigned char clearstring[MAX_SIZE_STRING];
	for(int i = 0 ; i < MAX_SIZE_STRING ; i++){
		clearstring[i] = 0;
	}
	for(unsigned char i = '1' ; i < '8' ; i++){
		set_string_variable(i,clearstring);
	}

	//
	// clear stack
	nestpointer = 0;
	for(int i = 0 ; i < MAX_SIZE_NEST ; i++){
		nests[i] = 0;
	}

	linenumber = 1;
}

Token getNextToken(){
	Token tok;
	//
	// clear old data
	for(int i = 0 ; i < (MAX_SIZE_STRING+2) ; i++){
		tok.definition[i] = 0;
	}
	tok.type = TOK_IS_EMPTY;

	//
	// fetch new token
	char deze = 0;
	int bufpos = 0;
	char isstring = 0;
	while(1){
		deze = buffer[cursorpos];
		if(deze==0){
			break;
		}
		cursorpos++;
		if(deze=='\n'){
			linenumber++;
		}
		if((deze==' '||deze=='\n'||deze=='\"'||deze=='\'')&&bufpos>0){
			break;
		}
		if(deze=='\"'||deze=='\''){
			again:
			deze = buffer[cursorpos++];
			if(deze=='\"'||deze=='\''){
				goto end;
			}
			if(deze=='\n'){
				linenumber++;
			}
			tok.definition[bufpos++] = deze;
			goto again;
			end:
			isstring = 1;
			break;
		}
		if(!(deze==' '||deze=='\n')){
			tok.definition[bufpos++] = deze;
		}
	}

	//
	// determinate token
	if(bufpos>0){
		tok.type = TOK_IS_KEYWORD;
	}
	if(isstring){
		tok.type = TOK_IS_STRING;
	}
	if(bufpos==1){
		tok.type = TOK_IS_VARIABLE;
	}
	if(tok.definition[bufpos-1]==':'){
		tok.type = TOK_IS_LABEL;
	}
	return tok;
}

int strcmp(char* a,char* b,int c){
	for(int i = 0 ; i < c ; i++){
		if(a[i]!=b[i]){return 0;}
	}
	return 1;
}

void walkTillEndOfLine(){
	char deze;
	while(1){
		deze = buffer[cursorpos];
		if(deze=='\n'||deze==0x00){
			break;
		}
		cursorpos++;
	}
}

int strlen(char* t){
	int result = 0;
	while(1){
		if(t[result]==0){
			break;
		}
		result++;
	}
	return result;
}

int nativeFunctionIsPresent(char* name,int strlen){
	int result = 0;
	for(int i = 0 ; i < functiontablepointer ; i++){
		if(functiontable[i].length==strlen){
			if(strcmp(name,functiontable[i].name,strlen)){
				result = i+1;
			}
		}
	}
	return result;
}

void introduceNativeFunction(char* name,unsigned long pointer,int strlen){
	if(functiontablepointer==MAX_NATIVE_FUNCTION){
		return;
	}
	if(nativeFunctionIsPresent(name,strlen)==0){
		functiontable[functiontablepointer].name = name;
		functiontable[functiontablepointer].length = strlen;
		functiontable[functiontablepointer].location = pointer;
		functiontablepointer++;
	}
}

void cls(){
	typedef void funct();
	funct *fc = (funct *)F_CLS; //browse
	fc();
}

void handleREM(){
	walkTillEndOfLine();
}

void handleECHO(){
	Token keyword = getNextToken();
	if(keyword.type==TOK_IS_STRING){
		msg(keyword.definition);
	}
	walkTillEndOfLine();
}

void handleEXIT(){
	msg("END OF BASIC");
	int mode = 1;
        __asm__ __volatile__ (
            "int $0x80"
            : "+a" (mode)
            );
	for(;;);
}

void main(){
	
	//
	// First, ask user for file to load
	typedef char *funct();
	funct *ft = (funct *)F_BROWSE; //browse
	filenamebuffer = ft();
	typedef char func(char *, unsigned char *);
	func *f = (func *)F_FREAD; // fread
	f(filenamebuffer, (unsigned char*)&buffer);
	filesizepointer = 1;
	while (1)
	{
		char deze = buffer[filesizepointer];
		char andere = 0;
		andere += deze & 0b01111111;
		buffer[filesizepointer] = andere;
		if (andere == 0)
		{
			break;
		}
		if(filesizepointer==BUFFER_INIT_SIZE){
			error("cannot find end of file");
		}
		filesizepointer++;
	}
	
	//
	// Then resetting all internal parameters
	clear_variables();

	//
	// Introduce native functions
	introduceNativeFunction("rem",(unsigned long)&handleREM,3);
	introduceNativeFunction("echo",(unsigned long)&handleECHO,4);
	introduceNativeFunction("exit",(unsigned long)&handleEXIT,4);

	//
	// Clear the screen
	cls();

	//
	// Then start interpeting
	Token keyword;
	again:
		keyword = getNextToken();
		if(keyword.type==TOK_IS_EMPTY){
			goto end;
		}
		if(keyword.type==TOK_IS_KEYWORD){
			int t = nativeFunctionIsPresent(keyword.definition,strlen(keyword.definition));
			if(t!=0){
				t--;
				if(functiontable[t].location==0){
					error("Native function is null");
				}
				typedef void funct();
				funct *fc = (funct *)functiontable[t].location; //browse
				fc();
			}else{
				error(keyword.definition);
			}
		}else if(keyword.type==TOK_IS_VARIABLE){

		}else if(keyword.type!=TOK_IS_LABEL){
			error("syntax error");
		}
	goto again;
	end:

	for(;;);
}