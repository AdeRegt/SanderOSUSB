#include "../kernel.h"

#define MAX_COMMAND_SIZE 10
#define MAX_COMMAND_STACK 5
#define MAX_COMMAND_COMMAND_SIZE 50

struct terminal_cmd{
    char name[MAX_COMMAND_SIZE];
    unsigned long addr;
} __attribute__((packed));

struct terminal_cmd term_commands[MAX_COMMAND_STACK];

char* read_line(int maxsize){
	char* buffer = (char*) malloc(maxsize);
	int i = 0;
	for(i = 0 ; i < maxsize ; i++){
		buffer[i] = (char)getch();
		putc(buffer[i]);
		if(buffer[i]=='\n'){
			break;
		}
	}
	buffer[i] = 0;
	return buffer;
}

void addTerminalCommand(char* cmd,unsigned long addr){
    int i = 0;
    for(int q = 0 ; q < MAX_COMMAND_STACK ; q++){
        if(term_commands[q].name[0]==0){
            i = q;
            break;
        }
    }
    for(int w = 0 ; w < MAX_COMMAND_SIZE ; w++){
        char t = cmd[w];
        term_commands[i].name[w] = t;
        if(t==0){
            break;
        }
    }
    term_commands[i].addr = addr;
    printf("terminal: id=%x command=%s address=%x \n",i+1,(char*)&term_commands[i].name,term_commands[i].addr);
}

void tty_help(char* argv){
    strlen(argv);
    printf("===CREDITS===\n");
    printf("Kernel created by \n - Sander de Regt\n - Shashwat Shagun\n - Johan Gericke\n - Daniel Mccarthy\n - Jark Clim\n - Pablo Narvaja\n - Nelson Cole\n");
    printf("\n");
    printf("===COMMANDS===\n");
    for(int i = 0 ; i < MAX_COMMAND_STACK ; i++){
        if(((char*)&term_commands[i].name)[0]!=0){
            printf(" - %s \n",(char*)&term_commands[i].name);
        }
    }
    printf("\n");
}

void tty_cd(char* argv){
    char* cmd;
    char* tuv = getcwd();
    if(strlen(tuv)==2){
        cmd = sprintf("%s%s",tuv,argv);
    }else{
        cmd = sprintf("%s/%s",tuv,argv);
    }
    setcwd(cmd);
}

void tty_dir(char* argv){
    char* cmd;
    char* tuv = getcwd();
    if(strlen(argv)){
        if(strlen(tuv)==2){
            cmd = sprintf("%s%s",tuv,argv);
        }else{
            cmd = sprintf("%s/%s",tuv,argv);
        }
    }else{
        cmd = tuv;
    }
    printf("DIR %s : %s \n",cmd,dir(cmd));
}

void tty_exec(char* argv){
    char* cmd;
    char* tuv = getcwd();
    if(strlen(argv)){
        if(strlen(tuv)==2){
            cmd = sprintf("%s%s",tuv,argv);
        }else{
            cmd = sprintf("%s/%s",tuv,argv);
        }
    }else{
        cmd = tuv;
    }
    int exparam = 0;
    for(int i = 0 ; i < strlen(cmd) ; i++){
        if(cmd[i]==' '){
            exparam = i+1;
        }
    }
    if(exparam==0){
        exparam = strlen(cmd);
    }
    cmd[exparam] = 0;
    unsigned char* buffer = (unsigned char*)0x2000;
	if(fexists((unsigned char *)cmd) && fread(cmd,buffer)){
        if(iself(buffer)){
            unsigned long gamma = loadelf(buffer);
            if(gamma!=0){
                cls();
                void* (*foo)() = (void*) gamma;
                foo();
            }
        }else{
            cls();
            void* (*foo)() = (void*) 0x2000;
            foo();
        }
    }
}

void tty_drive(char* argv){
    setcwd(sprintf("%c@",argv[0]));
}

void init_tty(){
    setForeGroundBackGround(7,0);
    addTerminalCommand("poweroff",(unsigned long)&poweroff);
    addTerminalCommand("help",(unsigned long)&tty_help);
    addTerminalCommand("cd",(unsigned long)&tty_cd);
    addTerminalCommand("dir",(unsigned long)&tty_dir);
    addTerminalCommand("drive",(unsigned long)&tty_drive);
    addTerminalCommand("exec",(unsigned long)&tty_exec);
}

volatile char* basepath;

void setpwd(volatile char* dw){
    basepath = dw;
}

char* getpwd(){
    return (char*)basepath;
}

int handle_tty_command(char* prompt){
    int f = 0;
    for(int i = 0 ; i < MAX_COMMAND_STACK ; i++){
        if(((char*)&term_commands[i].name)[0]){
            if(memcmp(prompt,(char*)&term_commands[i].name,strlen((char*)&term_commands[i].name))==0){
                int g = 0;
                for(int y = 0 ; y < strlen(prompt) ; y++){
                    if(prompt[y]==' '||prompt[y]=='\n'){
                        g = y;
                        g++;
                        break;
                    }
                }
                if(g==0){
                    g = strlen(prompt);
                }
                prompt[strlen(prompt)] = 0x00;
                prompt[strlen(prompt)+1] = 0x00;
                void* (*foo)(char*) = (void*) term_commands[i].addr;
                f = 1;
                foo((char*)(prompt+g));
            }
        }
    }
    if(!f){
        printf("No such command!\n");
        return 0;
    }
    return 1;
}

void tty_loop(){
    setpwd("@");
    while(1){
        printf("[%s]-> ",getcwd());
        char* prompt = read_line(MAX_COMMAND_COMMAND_SIZE);
        handle_tty_command(prompt);
        free(prompt);
    }
}